#include "Server.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include "MSTFactory.hpp"
#include "MSTAlgorithmType.hpp"

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
            std::cout << "Client connected\n";
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
    else if (command == "TotalWeight")
    {
        sendTotalWeight(client_socket, client_socket);
    }
    else if (command == "LongestDistance")
    {
        sendLongestDistance(client_socket, client_socket);
    }
    else if (command == "AverageDistance")
    {
        sendAverageDistance(client_socket, client_socket);
    }
    else if (command == "ShortestDistance")
    {
        int v1, v2;
        iss >> v1 >> v2;
        sendShortestDistance(client_socket, client_socket, v1, v2);
    }
}

void Server::addGraph(int client_id, int n)
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    graphs[client_id] = Graph(n);
    std::cout << "New graph created with " << n << " vertices\n";
}

void Server::addEdge(int client_id, int i, int j, int weight)
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    graphs[client_id].addEdge(i, j, weight);
    std::cout << "Added edge (" << i << ", " << j << ") with weight " << weight << "\n";
}

void Server::removeEdge(int client_id, int i, int j)
{
    std::lock_guard<std::mutex> lock(graph_mutex);
    graphs[client_id].removeEdge(i, j);
    std::cout << "Removed edge (" << i << ", " << j << ")\n";
}

void Server::solveMST(int client_id, int client_socket, const std::string &algorithm)
{
    std::lock_guard<std::mutex> lock(graph_mutex);

    MSTAlgorithmType algoType = stringToAlgorithmType(algorithm);
    MSTFactory factory;
    auto solver = factory.createSolver(algoType);

    if (solver)
    {
        mstResults[client_id] = solver->computeMST(graphs[client_id].getEdges(), graphs[client_id].getVertexCount());

        // Prepare the MST result to send back
        std::ostringstream response;
        response << "MST result:\n";
        for (const auto &[from, to, weight, id] : mstResults[client_id].mstEdges)
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

void Server::sendTotalWeight(int client_socket, int client_id)
{
    int weight = mstResults[client_id].totalWeight;
    send(client_socket, &weight, sizeof(weight), 0);
}

void Server::sendLongestDistance(int client_socket, int client_id)
{
    int longest = mstResults[client_id].longestDistance;
    send(client_socket, &longest, sizeof(longest), 0);
}

void Server::sendAverageDistance(int client_socket, int client_id)
{
    double average = mstResults[client_id].averageDistance;
    send(client_socket, &average, sizeof(average), 0);
}

void Server::sendShortestDistance(int client_socket, int client_id, int v1, int v2)
{
    int shortest = mstResults[client_id].shortestDistances.at(v1).at(v2);
    send(client_socket, &shortest, sizeof(shortest), 0);
}

int main()
{
    int port;
    std::cout << "Enter server port: ";
    std::cin >> port;

    Server server(port);
    server.start();

    return 0;
}
