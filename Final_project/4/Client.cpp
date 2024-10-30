// Client.cpp
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
    server_address.sin_addr.s_addr = INADDR_ANY; // Using localhost

    if (connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cerr << "Connection to server failed\n";
        return false;
    }
    return true;
}

void Client::sendRequest(const std::string &request)
{
    send(client_fd, request.c_str(), request.size(), 0);
}

std::string Client::receiveResponse()
{
    char buffer[1024] = {0};
    int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        return std::string(buffer);
    }
    return "";
}

// Main function for running the client
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

    while (true)
    {
        std::string command;
        std::cout << "Enter command (NewGraph, AddEdge, RemoveEdge, SolveMST, or quit): ";
        std::cin.ignore();
        std::getline(std::cin, command);

        if (command == "quit")
        {
            std::cout << "Exiting client.\n";
            break;
        }

        client.sendRequest(command);

        std::string response = client.receiveResponse();
        std::cout << "Server response:\n"
                  << response << "\n";
    }

    return 0;
}
