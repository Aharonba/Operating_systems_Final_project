#pragma once

#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <map>
#include <netinet/in.h>
#include "Graph.hpp"
#include "MSTFactory.hpp"
#include "MSTResult.hpp"

class Server
{
public:
    Server(int port);
    ~Server();

    void start();
    void stop();

private:
    int port;
    int server_fd;
    std::mutex graph_mutex;
    std::map<int, Graph> graphs;         // Store graphs by client ID or request ID
    std::map<int, MSTResult> mstResults; // Store MST results for each client

    void handleClient(int client_socket);
    void processRequest(int client_socket, const std::string &request);

    void addGraph(int client_id, int n);
    void addEdge(int client_id, int i, int j, int weight);
    void removeEdge(int client_id, int i, int j);
    void solveMST(int client_id, int client_socket, const std::string &algorithm);

    void sendTotalWeight(int client_socket, int client_id);
    void sendLongestDistance(int client_socket, int client_id);
    void sendAverageDistance(int client_socket, int client_id);
    void sendShortestDistance(int client_socket, int client_id, int v1, int v2);
};
