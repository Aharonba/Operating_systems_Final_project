#include "Client.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>

// Constructor: Initializes client with the server port number
Client::Client(int server_port)
    : server_port(server_port), client_fd(-1) {}

// Destructor: Ensures the socket is closed if it's still open
Client::~Client()
{
    if (client_fd >= 0)
    {
        close(client_fd);
    }
}

// Attempts to establish a connection to the server
bool Client::connectToServer()
{
    // Create a socket
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
    {
        std::cerr << "Socket creation error\n";
        return false;
    }

    // Set up server address structure for localhost
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // Connect to localhost

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cerr << "Connection to server failed\n";
        return false;
    }
    return true;
}

// Sends a request string to the server, ensuring data is sent immediately
void Client::sendRequest(const std::string &request)
{
    send(client_fd, request.c_str(), request.size(), 0);
    fsync(client_fd); // Ensure data is sent immediately
}

// Receives a response from the server
std::string Client::receiveResponse()
{
    int32_t responseSize = 0; // Size of the response
    int bytesReceived = recv(client_fd, &responseSize, sizeof(responseSize), 0);

    // Check if the response size was successfully received
    if (bytesReceived <= 0)
    {
        std::cerr << "Failed to receive response size\n";
        return "";
    }

    // Resize response string to expected size and read data into it
    std::string response(responseSize, '\0');
    bytesReceived = recv(client_fd, const_cast<char *>(response.data()), responseSize, 0);

    // Check if the full response was successfully received
    if (bytesReceived <= 0)
    {
        std::cerr << "Failed to receive full response\n";
        return "";
    }

    return response;
}

// Main function for running the client
int main()
{
    int port;
    std::cout << "Enter server port: ";
    std::cin >> port;

    // Initialize client and attempt to connect to the server
    Client client(port);
    if (!client.connectToServer())
    {
        std::cerr << "Failed to connect to server on port " << port << "\n";
        return 1;
    }

    std::cout << "Connected to server on port " << port << "\n";

    std::cin.ignore(); // Clear any remaining input buffer after reading port

    // Main loop for handling client commands
    while (true)
    {
        std::string command;
        std::cout << "Enter command (NewGraph, AddEdge, RemoveEdge, SolveMST, or quit): ";
        std::getline(std::cin, command);

        if (command == "quit")
        {
            std::cout << "Exiting client.\n";
            break;
        }

        // Send command to server
        client.sendRequest(command);

        // If command is "SolveMST", receive and display server response
        if (command.rfind("SolveMST", 0) == 0) // Check if command is SolveMST
        {
            std::string response = client.receiveResponse();
            std::cout << "Server response:\n"
                      << response << "\n";
        }
    }

    return 0;
}
