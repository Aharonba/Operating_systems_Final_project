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

#define NUM_THREADS 4 // Number of threads for LFP
// Declare `lfp` as a unique pointer to LFP (not constructed yet)
std::unique_ptr<LFP> lfp;

using namespace std;

// Global variables
std::atomic<bool> isRunning(true); // Flag for server running status

Server::Server(int port) : port(port), server_fd(-1) {}

Server::~Server()
{
    stop();
}

void Server::start()
{
    // Register signal handler for graceful shutdown
    signal(SIGINT, [](int)
           { isRunning = false; });

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
    memset(&address, 0, sizeof(address)); // Clear the struct
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
            std::cout << "Client with FD: " << client_socket << " connected." << std::endl;
            std::thread(&Server::handleClient, this, client_socket).detach();
            client_counter++;
        }
    }
}

void Server::stop()
{
    isRunning = false; // Set flag to stop accepting clients
    if (server_fd >= 0)
    {
        close(server_fd); // Close server socket
        server_fd = -1;
    }
    cout << "Server stopped.\n";
}

void Server::handleClient(int client_socket)
{
    char buffer[1024];
    int bytes_read;

    // Read and process client requests
    while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0';
        processRequest(client_socket, buffer);
    }

    // Close client socket after disconnecting
    close(client_socket);
    cout << "Client disconnected from FD: " << client_socket << "\n";
}

void Server::processRequest(int client_socket, const string &request)
{
    istringstream iss(request);
    string command;
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
        string algorithm;
        iss >> algorithm;
        MSTAlgorithmType algoType = stringToAlgorithmType(algorithm);
        solveMSTWithLF(client_socket, algoType);
    }
    else
    {
        cerr << "Unknown command received: " << command << "\n";
    }
}

void Server::addGraph(int client_id, int n)
{
    lock_guard<mutex> lock(graph_mutex);
    clients_graphs[client_id] = Graph(n);
    cout << "New graph created with " << n << " vertices for client " << client_id << "\n";
}

void Server::addEdge(int client_id, int i, int j, int weight)
{
    lock_guard<mutex> lock(graph_mutex);
    clients_graphs[client_id].addEdge(i, j, weight);
    cout << "Added edge (" << i << ", " << j << ") with weight " << weight << " for client " << client_id << "\n";
}

void Server::removeEdge(int client_id, int i, int j)
{
    lock_guard<mutex> lock(graph_mutex);
    clients_graphs[client_id].removeEdge(i, j);
    cout << "Removed edge (" << i << ", " << j << ") for client " << client_id << "\n";
}

void Server::solveMSTWithLF(int client_socket, MSTAlgorithmType algoType)
{
    auto it = clients_graphs.find(client_socket);
    if (it == clients_graphs.end())
    {
        cerr << "No graph found for client " << client_socket << "\n";
        return;
    }

    MSTFactory factory;
    auto solver = factory.createSolver(algoType);
    if (!solver)
    {
        cerr << "Invalid MST algorithm requested\n";
        return;
    }

    auto &graph = it->second;
    MSTResult mst = solver->computeMST(graph.getEdges(), graph.getVertexCount());

    for (const auto &[from, to, weight, id] : mst.mstEdges)
    {
        cout << "Edge from " << from << " to " << to << " with weight " << weight << "\n";
    }

    lfp->addTask([client_socket, mst]() { // Capture mst by value
        ostringstream response;
        response << "Client " << client_socket << " MST:\n";

        // Process MST edges
        for (const auto &[from, to, weight, id] : mst.mstEdges)
        {
            response << "Edge from " << from << " to " << to << " with weight " << weight << "\n";
        }

        // Append other MST details
        response << "Total weight: " << mst.totalWeight << "\n";
        response << "Average distance: " << mst.averageDistance << "\n";
        response << "Longest distance: " << mst.longestDistance << "\n";
        response << "Shortest paths in MST:\n";

        for (const auto &row : mst.shortestDistances)
        {
            for (const auto &dist : row.second)
            {
                response << "From " + std::to_string(row.first) + " to " + std::to_string(dist.first) +
                                ": " + std::to_string(dist.second) + "\n";
            }
        }

        std::string responseStr = response.str();
        int32_t responseSize = static_cast<int32_t>(responseStr.size());

        // Send response size to client
        send(client_socket, &responseSize, sizeof(responseSize), 0);

        // Send response content to client
        send(client_socket, responseStr.c_str(), responseSize, 0);

        std::cout << "Finished sending MST result to client " << client_socket << "\n";
    });
}

// Main function
int main()
{
    int port;
    std::cout << "Enter server port: ";
    std::cin >> port;

    lfp = std::make_unique<LFP>(NUM_THREADS); // Construct `LFP` instance with NUM_THREADS

    // Create and start the server
    Server server(port);
    server.start();

    return 0;
}
