#include "Server.hpp"
#include "MSTFactory.hpp"
#include "MSTAlgorithmType.hpp"
#include "PAO.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <thread>
#include <mutex>
#include <map>

// Global instance of the PAO pipeline
PAO *pao = nullptr;
std::atomic<int> clientCount{0}; // Counter to track connected clients

// Function to initialize the PAO pipeline
void initializePAO()
{
    std::vector<std::function<void(void *)>> functions = {
        [](void *taskPtr)
        {
            Triple *task = static_cast<Triple *>(taskPtr);
            if (task->mstGraph)
            {
                task->msg += "Total weight of MST: " + std::to_string(task->mstGraph->totalWeight) + "\n";
            }
        },
        [](void *taskPtr)
        {
            Triple *task = static_cast<Triple *>(taskPtr);
            if (task->mstGraph)
            {
                task->msg += "Longest path in MST: " + std::to_string(task->mstGraph->longestDistance) + "\n";
            }
        },
        [](void *taskPtr)
        {
            Triple *task = static_cast<Triple *>(taskPtr);
            if (task->mstGraph)
            {
                task->msg += "Average distance in MST: " + std::to_string(task->mstGraph->averageDistance) + "\n";
            }
        },
        [](void *taskPtr)
        {
            Triple *task = static_cast<Triple *>(taskPtr);
            if (task->mstGraph)
            {
                task->msg += "Shortest paths in MST:\n";
                for (const auto &row : task->mstGraph->shortestDistances)
                {
                    for (const auto &dist : row.second)
                    {
                        task->msg += "From " + std::to_string(row.first) + " to " + std::to_string(dist.first) +
                                     ": " + std::to_string(dist.second) + "\n";
                    }
                }
            }
        },
        [](void *taskPtr)
        {
            Triple *task = static_cast<Triple *>(taskPtr);
            std::ostringstream response;
            for (const auto &[from, to, weight, id] : task->mstGraph->mstEdges)
            {
                response << "Edge from " << from << " to " << to << " with weight " << weight << "\n";
            }

            // Add any final pipeline data here
            response << "\nFinal pipeline data:\n";
            // pipelineData is a string containing the final data
            response << task->msg;
            // Convert to a single string and send it
            std::string responseStr = response.str();
            int32_t responseSize = responseStr.size();

            // Send the size and then the combined response in one go
            send(task->clientFd, &responseSize, sizeof(responseSize), 0);
            send(task->clientFd, responseStr.c_str(), responseSize, 0);
        }};

    pao = new PAO(functions);
    pao->initializeWorkers();
}

Server::Server(int port) : port(port), server_fd(-1) {}

Server::~Server()
{
    stop();
}

void Server::start()
{
    static int client_counter = 1;

    // Create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "Failed to create socket\n";
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Bind failed\n";
        close(server_fd);
        return;
    }

    if (listen(server_fd, 5) < 0)
    {
        std::cerr << "Listen failed\n";
        close(server_fd);
        return;
    }

    std::cout << "Server listening on port " << port << "\n";

    while (true)
    {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket >= 0)
        {
            // Increment client count and check if the limit is reached
            if (++clientCount > 2)
            {
                std::cout << "Server reached the maximum number of clients. Stopping.\n";
                break; // Exit loop to stop accepting new clients
            }

            std::cout << "Client with FD: " << client_socket << " connected." << std::endl;
            clientThreads.push_back(std::thread(&Server::handleClient, this, client_socket));
            client_counter++;
        }
    }

    // After accepting all clients, join all threads
    for (auto &t : clientThreads)
    {
        if (t.joinable())
        {
            t.join(); // Wait for each thread to finish
        }
    }
}

void Server::stop()
{
    if (server_fd >= 0)
    {
        close(server_fd);
        server_fd = -1;
    }

    // Join all threads before stopping
    for (auto &t : clientThreads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
}

void Server::handleClient(int client_socket)
{
    char buffer[1024];
    int bytes_read;

    while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0';
        processRequest(client_socket, buffer);
    }

    close(client_socket);
}

void Server::processRequest(int client_socket, const std::string &request)
{
    std::istringstream iss(request);
    std::string command;
    iss >> command;

    if (command == "NewGraph")
    {
        int n;
        iss >> n;
        addGraph(client_socket, n);
    }
    else if (command == "AddEdge")
    {
        int i, j, weight;
        iss >> i >> j >> weight;
        addEdge(client_socket, i, j, weight);
    }
    else if (command == "RemoveEdge")
    {
        int i, j;
        iss >> i >> j;
        removeEdge(client_socket, i, j);
    }
    else if (command == "SolveMST")
    {
        std::string algorithm;
        iss >> algorithm;
        MSTAlgorithmType algoType = stringToAlgorithmType(algorithm);
        solveMSTWithPipeline(client_socket, algoType, algorithm);
    }
}

void Server::addGraph(int client_id, int n)
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    clientGraphs[client_id] = Graph(n);
    std::cout << "New graph created with " << n << " vertices for client " << client_id << "\n";
}

void Server::addEdge(int client_id, int i, int j, int weight)
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    clientGraphs[client_id].addEdge(i, j, weight);
    std::cout << "Added edge (" << i << ", " << j << ") with weight " << weight << " for client " << client_id << "\n";
}

void Server::removeEdge(int client_id, int i, int j)
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    clientGraphs[client_id].removeEdge(i, j);
    std::cout << "Removed edge (" << i << ", " << j << ") for client " << client_id << "\n";
}

void Server::solveMSTWithPipeline(int client_socket, MSTAlgorithmType algoType, const std::string algorithm)
{
    std::lock_guard<std::mutex> lock(graph_mutex);

    Graph *clientGraph = &clientGraphs[client_socket];
    if (!clientGraph)
    {
        std::cerr << "No graph found for client " << client_socket << "\n";
        return;
    }

    MSTFactory factory;
    auto solver = factory.createSolver(algoType);
    if (solver)
    {
        mstResults[client_socket] = solver->computeMST(clientGraphs[client_socket].getEdges(), clientGraphs[client_socket].getVertexCount());

        auto task = std::make_unique<Triple>(Triple{&mstResults[client_socket], "MST created using " + algorithm + " algorithm.\n", client_socket});
        clientTasks[client_socket] = std::move(task);

        pao->enqueueTask(static_cast<void *>(clientTasks[client_socket].get()));
        std::cout << "Added MST task to pipeline for client " << client_socket << "\n";
    }
}

// Main function
int main()
{
    int port;
    std::cout << "Enter server port: ";
    std::cin >> port;

    // Initialize the PAO pipeline
    initializePAO();

    // Create and start the server
    Server server(port);
    server.start();

    delete pao;

    return 0;
}