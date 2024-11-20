#pragma once

#include <string>

class Client
{
public:
    Client(int server_port);
    ~Client();

    bool connectToServer();
    void sendRequest(const std::string &request);
    std::string receiveResponse();  // Receives a string response

private:
    int server_port;
    int client_fd;
};
