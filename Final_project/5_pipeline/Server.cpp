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

/**
 * @brief Initializes the PAO pipeline with task functions for MST processing.
 */
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

            response << "\nFinal pipeline data:\n";
            response << task->msg;
            std::string responseStr = response.str();
            int32_t responseSize = responseStr.size();

            send(task->clientFd, &responseSize, sizeof(responseSize), 0);
            send(task->clientFd, responseStr.c_str(), responseSize, 0);

            safePrint("The pipeline process has completed its work (: \n");
        }};

    pao = new PAO(functions);
    pao->initializeWorkers();
}

/**
 * @brief Server constructor that initializes the port.
 * @param port The port on which the server listens.
 */
Server::Server(int port) : port(port), server_fd(-1) {}

/**
 * @brief Destructor that stops the server and cleans up resources.
 */
Server::~Server()
{
    stop();
}

/**
 * @brief Starts the server and listens for client connections.
 */
void Server::start()
{
    static int client_counter = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        safePrint("Failed to create socket\n");
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
        safePrint("Bind failed\n");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 5) < 0)
    {
        safePrint("Listen failed\n");
        close(server_fd);
        return;
    }

    safePrint("Server listening on port " + std::to_string(port));

    while (true)
    {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket >= 0)
        {
            // if (++clientCount > 2)
            // {
            //     safePrint("Server reached the maximum number of clients. Stopping\n");
            //     break;
            // }
            safePrint("Client with FD: " + std::to_string(client_socket) + " connected.");
            clientThreads.push_back(std::thread(&Server::handleClient, this, client_socket));
            client_counter++;
        }
    }

    for (auto &t : clientThreads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
}

/**
 * @brief Stops the server and cleans up resources.
 */
void Server::stop()
{
    if (server_fd >= 0)
    {
        close(server_fd);
        server_fd = -1;
    }

    for (auto &t : clientThreads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
}

/**
 * @brief Handles communication with a client.
 * @param client_socket The client's socket file descriptor.
 */
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

/**
 * @brief Processes client requests.
 * @param client_socket The client's socket file descriptor.
 * @param request The request string from the client.
 */
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

/**
 * @brief Creates a new graph for the client.
 * @param client_id The client ID.
 * @param n The number of vertices in the graph.
 */
void Server::addGraph(int client_id, int n)
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    clientGraphs[client_id] = Graph(n);
    safePrint("New graph created with " + std::to_string(n) + " vertices for client " + std::to_string(client_id));
}

/**
 * @brief Adds an edge to the client's graph.
 * @param client_id The client ID.
 * @param i The first vertex.
 * @param j The second vertex.
 * @param weight The edge weight.
 */
void Server::addEdge(int client_id, int i, int j, int weight)
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    clientGraphs[client_id].addEdge(i, j, weight);
    safePrint("Added edge (" + std::to_string(i) + ", " + std::to_string(j) + ") with weight " + std::to_string(weight) + " for client " + std::to_string(client_id));
}

/**
 * @brief Removes an edge from the client's graph.
 * @param client_id The client ID.
 * @param i The first vertex.
 * @param j The second vertex.
 */
void Server::removeEdge(int client_id, int i, int j)
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    clientGraphs[client_id].removeEdge(i, j);
    safePrint("Removed edge (" + std::to_string(i) + ", " + std::to_string(j) + ") for client " + std::to_string(client_id));
}

/**
 * @brief Solves the MST and processes the result using the PAO pipeline.
 * @param client_socket The client's socket file descriptor.
 * @param algoType The MST algorithm to use.
 * @param algorithm The name of the algorithm.
 */
void Server::solveMSTWithPipeline(int client_socket, MSTAlgorithmType algoType, const std::string algorithm)
{
    std::lock_guard<std::mutex> graphLock(graph_mutex);
    std::lock_guard<std::mutex> resultsLock(mstResultsMutex);
    std::lock_guard<std::mutex> tasksLock(clientTasksMutex);

    safePrint("**solveMSTWithPipeline:**\n");

    if (clientGraphs.find(client_socket) == clientGraphs.end())
    {
        safePrint("No graph found for client " + std::to_string(client_socket));
        return;
    }

    mstResults[client_socket] = MSTResult();
    clientTasks.erase(client_socket);

    MSTFactory factory;
    auto solver = factory.createSolver(algoType);

    if (solver)
    {
        std::lock_guard<std::mutex> clientLock(clientMutexes[client_socket]);

        auto mst = solver->computeMST(clientGraphs[client_socket].getEdges(), clientGraphs[client_socket].getVertexCount());
        mstResults[client_socket] = mst;

        safePrint("MST computed successfully for client " + std::to_string(client_socket));

        auto task = std::make_unique<Triple>(Triple{&mstResults[client_socket], "MST created using " + algorithm + " algorithm.\n", client_socket});

        clientTasks[client_socket] = std::move(task);
        pao->enqueueTask(static_cast<void *>(clientTasks[client_socket].get()));

        safePrint("Added MST task to pipeline for client " + std::to_string(client_socket));
    }
    else
    {
        safePrint("Failed to create solver for " + algorithm + " algorithm.");
    }
}

/**
 * @brief Main entry point for the server application.
 */
int main()
{
    int port;
    safePrint("Enter server port: ");
    std::cin >> port;

    initializePAO();

    Server server(port);
    server.start();

    delete pao;

    return 0;
}
