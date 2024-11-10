#pragma once

#include <string>

class Client
{
public:
    Client(int server_port); 
    ~Client();

    bool connectToServer();
    void sendRequest(const std::string &request);
    std::string receiveResponse(); 

private:
    const std::string server_ip = "127.0.0.1"; 
    int server_port;
    int client_fd;
};
