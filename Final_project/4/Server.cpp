// Server.cpp
#include "Server.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include "MSTFactory.hpp"
#include "MSTAlgorithmType.hpp"

// Constructor to initialize server with specified port
Server::Server(int port) : port(port), server_fd(-1) {}

Server::~Server()
{
    stop();
}

// Start the server and listen for incoming client connections
void Server::start()
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "Failed to create socket\n";
        return;
    }

    // Set SO_REUSEADDR to allow port reuse in case of server restarts
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "setsockopt(SO_REUSEADDR) failed\n";
        close(server_fd);
        return;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket to the port and start listening for connections
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

    // Accept incoming client connections in a loop
    while (true)
    {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket >= 0)
        {
            static int counter = 1;
            std::cout << "Client " << counter << " connected\n";
            std::thread(&Server::handleClient, this, client_socket).detach();
            counter++;
        }
    }
}

// Stop the server and close the socket
void Server::stop()
{
    if (server_fd >= 0)
    {
        close(server_fd);
        server_fd = -1;
    }
}

// Handle communication with a single client
void Server::handleClient(int client_socket)
{
    char buffer[1024] = {0};
    int bytes_read;

    // Continuously read client requests
    while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0';
        processRequest(client_socket, buffer);
    }
    close(client_socket);
}

// Process client requests and route to appropriate methods
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
        solveMST(client_socket, client_socket, algorithm);
    }
}

// Create a new graph for the client
void Server::addGraph(int client_id, int n)
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    graphs[client_id] = Graph(n);
    std::cout << "New graph created with " << n << " vertices\n";
}

// Add an edge to the client's graph
void Server::addEdge(int client_id, int i, int j, int weight)
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    graphs[client_id].addEdge(i, j, weight);
    std::cout << "Added edge (" << i << ", " << j << ") with weight " << weight << "\n";
}

// Remove an edge from the client's graph
void Server::removeEdge(int client_id, int i, int j)
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    graphs[client_id].removeEdge(i, j);
    std::cout << "Removed edge (" << i << ", " << j << ")\n";
}

// Solve MST for the client's graph and send results back
void Server::solveMST(int client_id, int client_socket, const std::string &algorithm)
{
    std::lock_guard<std::mutex> lock(graph_mutex);

    MSTAlgorithmType algoType = stringToAlgorithmType(algorithm);
    MSTFactory factory;
    auto solver = factory.createSolver(algoType);

    if (solver)
    {
        auto mst = solver->computeMST(graphs[client_id].getEdges(), graphs[client_id].getVertexCount());

        // Prepare the MST result to send back
        std::ostringstream response;
        response << "MST result:\n";
        for (const auto &[from, to, weight, id] : mst)
        {
            response << "Edge from " << from << " to " << to << " with weight " << weight << "\n";
        }

        // Send message length and data
        std::string responseStr = response.str();
        int32_t responseSize = responseStr.size();
        send(client_socket, &responseSize, sizeof(responseSize), 0);
        send(client_socket, responseStr.c_str(), responseSize, 0);
    }
    else
    {
        std::string error = "Error: Unsupported MST algorithm\n";
        int32_t errorSize = error.size();
        send(client_socket, &errorSize, sizeof(errorSize), 0);
        send(client_socket, error.c_str(), errorSize, 0);
    }
}

// Main function to start server
int main()
{
    int port;
    std::cout << "Enter server port: ";
    std::cin >> port;

    Server server(port);
    server.start();

    return 0;
}
