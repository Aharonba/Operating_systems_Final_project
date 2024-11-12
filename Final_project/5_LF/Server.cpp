#include "Server.hpp"
#include "MSTFactory.hpp"
#include "MSTAlgorithmType.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <thread>
#include <mutex>
#include <map>
#include <atomic>
#include <vector>

std::atomic<bool> isRunning{true};
std::atomic<int> clientCount{0}; // Counter to track connected clients

#define NUM_THREADS 4 // Number of threads for LFP
std::unique_ptr<LFP> lfp;

std::mutex coutMutex; // Ensure this is global or static within the file

void threadSafePrint(const std::ostringstream &oss)
{
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << oss.str() << std::endl;
}

// Constructor
Server::Server(int port) : port(port), server_fd(-1) {}

// Destructor
Server::~Server()
{
    stop();
}

void Server::start()
{
    signal(SIGINT, [](int)
           { isRunning = false; });

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::ostringstream oss;
        oss << "Failed to create socket";
        threadSafePrint(oss);
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::ostringstream oss;
        oss << "Bind failed";
        threadSafePrint(oss);
        close(server_fd);
        return;
    }

    if (listen(server_fd, 5) < 0)
    {
        std::ostringstream oss;
        oss << "Listen failed";
        threadSafePrint(oss);
        close(server_fd);
        return;
    }

    std::ostringstream oss;
    oss << "Server listening on port " << port;
    threadSafePrint(oss);

    while (isRunning)
    {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket >= 0)
        {
            // Increment client count and check if the limit is reached
            if (++clientCount > 2)
            {
                std::ostringstream oss;
                oss << "Server reached the maximum number of clients. Stopping.";
                threadSafePrint(oss);
                break; // Exit loop to stop accepting new clients
            }

            std::ostringstream oss;
            oss << "Client with FD: " << client_socket << " connected.";
            threadSafePrint(oss);

            std::lock_guard<std::mutex> lock(client_mutex);
            client_sockets.push_back(client_socket);

            client_threads.emplace_back(&Server::handleClient, this, client_socket);
        }
    }

    std::ostringstream re;
    re << "Server shutting down after accepting 2 clients.";
    threadSafePrint(re);

    // Wait for all client threads to finish
    for (auto &thread : client_threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    // closeAllConnections(); // Close all client connections

    stop(); // Stop the server
    std::ostringstream stopMessage;
    stopMessage << "Server stopped, exiting the program.";
    threadSafePrint(stopMessage);

}

void Server::stop()
{
    isRunning = false;
    if (server_fd >= 0)
    {
        close(server_fd);
        server_fd = -1;
    }
    std::ostringstream oss;
    oss << "Server stopped.";
    threadSafePrint(oss);
}

void Server::closeAllConnections()
{
    std::lock_guard<std::mutex> lock(client_mutex);
    for (int client_socket : client_sockets)
    {
        close(client_socket);
        std::ostringstream oss;
        oss << "Closed connection with client FD: " << client_socket;
        threadSafePrint(oss);
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
    std::ostringstream oss;
    oss << "Client disconnected from FD: " << client_socket;
    threadSafePrint(oss);
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
        solveMSTWithLF(client_socket, algoType);
    }
    else
    {
        std::ostringstream oss;
        oss << "Unknown command received: " << command;
        threadSafePrint(oss);
    }
}

void Server::addGraph(int client_id, int n)
{
    std::lock_guard<std::mutex> lock(clientsGraphsMutex);
    clients_graphs[client_id] = Graph(n);
    std::ostringstream oss;
    oss << "New graph created with " << n << " vertices for client " << client_id;
    threadSafePrint(oss);
}

void Server::addEdge(int client_id, int i, int j, int weight)
{
    std::lock_guard<std::mutex> lock(clientsGraphsMutex);
    clients_graphs[client_id].addEdge(i, j, weight);
    std::ostringstream oss;
    oss << "Added edge (" << i << ", " << j << ") with weight " << weight << " for client " << client_id;
    threadSafePrint(oss);
}

void Server::removeEdge(int client_id, int i, int j)
{
    std::lock_guard<std::mutex> lock(clientsGraphsMutex);
    clients_graphs[client_id].removeEdge(i, j);
    std::ostringstream oss;
    oss << "Removed edge (" << i << ", " << j << ") for client " << client_id;
    threadSafePrint(oss);
}

void Server::solveMSTWithLF(int client_socket, MSTAlgorithmType algoType)
{
    {
        std::lock_guard<std::mutex> lock(clientsGraphsMutex);

        auto it = clients_graphs.find(client_socket);
        if (it == clients_graphs.end())
        {
            std::ostringstream oss;
            oss << "No graph found for client " << client_socket;
            threadSafePrint(oss);
            return;
        }

        MSTFactory factory;
        auto solver = factory.createSolver(algoType);
        if (!solver)
        {
            std::ostringstream oss;
            oss << "Invalid MST algorithm requested";
            threadSafePrint(oss);
            return;
        }

        auto graph = it->second;
        MSTResult mst = solver->computeMST(graph.getEdges(), graph.getVertexCount());

        lfp->addTask([client_socket, mst]()
                     {
            std::ostringstream response;
            response << "Client " << client_socket << " MST:\n";

            for (const auto &[from, to, weight, id] : mst.mstEdges)
            {
                response << "Edge from " << from << " to " << to << " with weight " << weight << "\n";
            }

            response << "Total weight: " << mst.totalWeight << "\n";
            response << "Average distance: " << mst.averageDistance << "\n";
            response << "Longest distance: " << mst.longestDistance << "\n";
            response << "Shortest paths in MST:\n";

            for (const auto &row : mst.shortestDistances)
            {
                for (const auto &dist : row.second)
                {
                    response << "From " + std::to_string(row.first) + " to " + std::to_string(dist.first) + ": " + std::to_string(dist.second) + "\n";
                }
            }

            std::string responseStr = response.str();
            int32_t responseSize = responseStr.size();

            send(client_socket, &responseSize, sizeof(responseSize), 0);
            send(client_socket, responseStr.c_str(), responseSize, 0); });
    }
}

int main()
{
    int port;
    std::ostringstream oss;
    oss << "Enter server port: ";
    threadSafePrint(oss);

    std::cin >> port;
    lfp = std::make_unique<LFP>(NUM_THREADS);

    Server server(port);
    server.start();

    std::ostringstream ror;
    ror << "main\n";
    threadSafePrint(ror);

    return 0;
}