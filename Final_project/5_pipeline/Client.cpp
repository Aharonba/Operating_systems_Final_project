#include "Client.hpp"
#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>

// Client constructor that takes the server port number
Client::Client(int server_port)
    : server_port(server_port), client_fd(-1) {}

// Destructor to close the client socket
Client::~Client()
{
    if (client_fd >= 0)
    {
        close(client_fd);  // Close the socket if it was opened
    }
}

// Connects the client to the server using the given port
bool Client::connectToServer()
{
    client_fd = socket(AF_INET, SOCK_STREAM, 0);  // Create a TCP socket
    if (client_fd < 0)
    {
        std::cerr << "Socket creation error\n";
        return false;
    }

    sockaddr_in server_address{};  
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);  // Set server port
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");  // Set server IP address

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cerr << "Connection to server failed\n";
        return false;
    }
    return true;
}

// Sends a request string to the server
void Client::sendRequest(const std::string &request)
{
    ssize_t sentBytes = send(client_fd, request.c_str(), request.size(), 0);  // Send request data
    if (sentBytes != static_cast<ssize_t>(request.size()))  // Check if all bytes were sent
    {
        std::cerr << "Warning: Only " << sentBytes << " bytes of " << request.size() << " were sent.\n";
    }
    fsync(client_fd);  // Ensure the data is written to the socket
}

// Receives the server's response as a string
std::string Client::receiveResponse()
{
    int32_t responseSize = 0;
    int bytesReceived = recv(client_fd, &responseSize, sizeof(responseSize), 0);  

    if (bytesReceived <= 0)
    {
        std::cerr << "Failed to receive response size\n";
        return "";
    }

    std::string response(responseSize, '\0');  // Prepare a string to hold the full response
    int totalBytesReceived = 0;

    // Receive the full response data
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

// Main function to handle user input and client-server communication
int main()
{
    int port;
    std::cout << "Enter server port: ";
    std::cin >> port;  // User input for server port

    Client client(port);
    if (!client.connectToServer())  // Try to connect to the server
    {
        std::cerr << "Failed to connect to server on port " << port << "\n";
        return 1;
    }

    std::cout << "Connected to server on port " << port << "\n";
    std::cin.ignore();  // Ignore the newline character left by std::cin

    while (true)
    {
        std::string command;
        std::cout << "Enter command (NewGraph, AddEdge, RemoveEdge, SolveMST Prim, SolveMST Kruskal): ";
        std::getline(std::cin, command);  // Read the user command

        if (command == "quit")
        {
            std::cout << "Exiting client.\n";
            break;  // Exit the loop if the user types "quit"
        }

        client.sendRequest(command);  // Send the command to the server

        // If the command involves solving MST, expect a response and display it
        if (command == "SolveMST Prim" || command == "SolveMST Kruskal")
        {
            std::string response = client.receiveResponse();  // Receive the response from the server
            std::cout << "The description of the mst:\n"
                      << response << "\n";  // Display the response
        }
    }
    return 0;  
}
