// Server.cpp
#include "Server.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include "MSTFactory.hpp"       // For MST algorithm selection
#include "MSTAlgorithmType.hpp" // For MSTAlgorithmType and stringToAlgorithmType()

Server::Server(int port) : port(port), server_fd(-1) {}

Server::~Server()
{
    stop();
}

void Server::start()
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "Failed to create socket\n";
        return;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Bind failed\n";
        return;
    }

    if (listen(server_fd, 5) < 0)
    {
        std::cerr << "Listen failed\n";
        return;
    }

    std::cout << "Server listening on port " << port << "\n";

    while (true)
    {
        int client_socket = accept(server_fd, nullptr, nullptr);
        if (client_socket >= 0)
        {
            std::thread(&Server::handleClient, this, client_socket).detach();
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
}

void Server::handleClient(int client_socket)
{
    char buffer[1024] = {0};
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
        int n, m;
        iss >> n >> m;
        addGraph(client_socket, n, m);
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
        solveMST(client_socket, client_socket, algorithm); // Pass client_socket as an argument
    }
}

void Server::addGraph(int client_id, int n, int m)
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    graphs[client_id] = Graph(n); // Initialize graph with `n` vertices
}

void Server::addEdge(int client_id, int i, int j, int weight)
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    graphs[client_id].addEdge(i, j, weight);
}

void Server::removeEdge(int client_id, int i, int j)
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    graphs[client_id].removeEdge(i, j);
}

void Server::solveMST(int client_id, int client_socket, const std::string &algorithm)
{
    std::lock_guard<std::mutex> lock(graph_mutex);

    // Convert the algorithm string to MSTAlgorithmType
    MSTAlgorithmType algoType = stringToAlgorithmType(algorithm);

    // Use MSTFactory to create an appropriate MST solver
    MSTFactory factory;
    auto solver = factory.createSolver(algoType);
    if (solver)
    {
        auto mst = solver->computeMST(graphs[client_id].getEdges(), graphs[client_id].getVertexCount());

        // Serialize and send the MST result back to the client
        std::ostringstream response;
        response << "MST result:\n";
        for (const auto &[from, to, weight, id] : mst)
        {
            response << "Edge from " << from << " to " << to << " with weight " << weight << "\n";
        }
        send(client_socket, response.str().c_str(), response.str().size(), 0);
    }
    else
    {
        std::string error = "Error: Unsupported MST algorithm\n";
        send(client_socket, error.c_str(), error.size(), 0);
    }
}

// Main function for running the server
int main()
{
    int port;
    std::cout << "Enter server port: ";
    std::cin >> port;

    Server server(port);
    server.start();

    return 0;
}
