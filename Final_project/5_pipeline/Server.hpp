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
#include "PAO.hpp"

// Task structure to represent each client request in the pipeline
struct Triple
{
    MSTResult *mstGraph; // Pointer to the MSTResult object
    std::string msg;     // Message string to accumulate results
    int clientFd;        // Client's file descriptor to send final results
};

// Global instance of the PAO pipeline
PAO *pao = nullptr;
std::atomic<int> clientCount{0}; // Counter to track connected clients

// Mutex for safe printing
std::mutex cout_mutex;

// Function to safely print messages to std::cout
void safePrint(const std::string &msg)
{
    std::lock_guard<std::mutex> lock(cout_mutex); // Lock the mutex
    std::cout << msg << std::endl;                // Safe printing
}

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

    std::unordered_map<int, std::mutex> clientMutexes; // Mutex for each client
    std::mutex graph_mutex;                            // Mutex for accessing graphs
    std::mutex mstResultsMutex;                        // Mutex for accessing mstResults
    std::mutex clientTasksMutex;                       // Mutex for accessing clientTasks

    // Maps to store client-specific data
    std::map<int, Graph> clientGraphs;                  // Graphs by client ID
    std::map<int, MSTResult> mstResults;                // Msts by client ID
    std::map<int, std::unique_ptr<Triple>> clientTasks; // Tasks by client ID, using unique_ptr to manage memory
    std::vector<std::thread> clientThreads;             // Stores client threads

    void handleClient(int client_socket);                               // Processes client connections
    void processRequest(int client_socket, const std::string &request); // Handles client requests

    // MST-related functions
    void addGraph(int client_id, int n);                                                                  // Adds a new graph for a client
    void addEdge(int client_id, int i, int j, int weight);                                                // Adds an edge
    void removeEdge(int client_id, int i, int j);                                                         // Removes an edge
    void solveMSTWithPipeline(int client_socket, MSTAlgorithmType algoType, const std::string algorithm); // Solves MST and passes task to PAO
};