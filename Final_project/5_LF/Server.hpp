#pragma once
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <map>
#include <netinet/in.h>
#include <atomic>
#include <memory>
#include "Graph.hpp"
#include "MSTFactory.hpp"
#include "MSTResult.hpp"
#include "LFP.hpp"

// Task structure to represent each client request in the pipeline
struct Triple
{
    MSTResult *mstGraph; // Pointer to the MSTResult object
    std::string msg;     // Message string to accumulate results
    int clientFd;        // Client's file descriptor to send final results
};

class Server
{
public:
    Server(int port);
    ~Server();

    void start(); // Starts the server
    void stop();  // Stops the server

private:
    int port;
    int server_fd;
    std::mutex client_mutex;                 // Mutex for synchronizing access to client_sockets and client_threads
    std::mutex clientsGraphsMutex;           // Mutex for synchronizing access to clients_graphs
    std::atomic<int> client_id_counter{1};   // Counter to generate unique client IDs
    std::vector<int> client_sockets;         // Vector to store connected client sockets
    std::vector<std::thread> client_threads; // Vector to store client handler threads

    // Maps to store client-specific data
    std::map<int, Graph> clients_graphs; // Graphs by client ID
    std::map<int, MSTResult> mstResults; // Msts by client ID
    std::map<int, Triple *> clientTasks; // Tasks by client ID

    void handleClient(int client_socket);                               // Processes client connections
    void processRequest(int client_socket, const std::string &request); // Handles client requests
    void closeAllConnections();

    // MST-related functions
    void addGraph(int client_id, int n);                   // Adds a new graph for a client
    void addEdge(int client_id, int i, int j, int weight); // Adds an edge
    void removeEdge(int client_id, int i, int j);          // Removes an edge
    void solveMSTWithLF(int client_id, MSTAlgorithmType algoType);

    // Send results to client
    void sendTotalWeight(int client_socket, int client_id);
    void sendLongestDistance(int client_socket, int client_id);
    void sendAverageDistance(int client_socket, int client_id);
    void sendShortestDistance(int client_socket, int client_id, int v1, int v2);
};

// Global flag for server running status
extern std::atomic<bool> isRunning;

