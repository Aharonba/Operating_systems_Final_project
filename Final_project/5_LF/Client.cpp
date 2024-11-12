#include "Client.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>

Client::Client(int server_port)
    : server_port(server_port), client_fd(-1) {}

Client::~Client()
{
    if (client_fd >= 0)
    {
        close(client_fd);
    }
}

bool Client::connectToServer()
{
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
    {
        std::cerr << "Socket creation error\n";
        return false;
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cerr << "Connection to server failed\n";
        return false;
    }
    return true;
}

void Client::sendRequest(const std::string &request)
{
    ssize_t sentBytes = send(client_fd, request.c_str(), request.size(), 0);
    if (sentBytes != static_cast<ssize_t>(request.size()))
    {
        std::cerr << "Warning: Only " << sentBytes << " bytes of " << request.size() << " were sent.\n";
    }
    fsync(client_fd);
}

std::string Client::receiveResponse()
{
    int32_t responseSize = 0;
    int bytesReceived = recv(client_fd, &responseSize, sizeof(responseSize), 0);

    if (bytesReceived <= 0)
    {
        std::cerr << "Failed to receive response size\n";
        return "";
    }

    std::string response(responseSize, '\0');
    int totalBytesReceived = 0;

    while (totalBytesReceived < responseSize)
    {
        bytesReceived = recv(client_fd, &response[totalBytesReceived], responseSize - totalBytesReceived, 0);
        if (bytesReceived <= 0)
        {
            std::cerr << "Failed to receive full response\n";
            return "";
        }
        totalBytesReceived += bytesReceived;
    }

    return response;
}

int main()
{
    int port;
    std::cout << "Enter server port: ";
    std::cin >> port;

    Client client(port);
    if (!client.connectToServer())
    {
        std::cerr << "Failed to connect to server on port " << port << "\n";
        return 1;
    }

    std::cout << "Connected to server on port " << port << "\n";
    std::cin.ignore();

    while (true)
    {

        std::string command;
        std::cout << "Enter command (NewGraph, AddEdge, RemoveEdge, SolveMST Prim, SolveMST Kruskal): ";
        std::getline(std::cin, command);

        if (command == "quit")
        {
            std::cout << "Exiting client.\n";
            break;
        }

        client.sendRequest(command);

        if (command.substr(0, 8) == "SolveMST")
        {
            std::cout << "**\n";
            // Expect a string response for MST commands
            std::string response = client.receiveResponse();
            std::cout << "The description of the mst:\n"
                      << response << " **" << "\n";
        }
    }
    return 0;
}