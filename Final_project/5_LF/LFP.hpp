#ifndef LFP_HPP
#define LFP_HPP

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>

using namespace std;

class LFP
{
private:
    void taskProcessor(int workerId);  // Worker function to process tasks
    vector<thread> workerThreads;      // Vector to store worker threads
    queue<function<void()>> taskQueue; // Queue to store tasks
    mutex queueMutex;                  // Mutex to protect the tasks queue
    mutex shutdownMutex;               // Mutex to protect the shutdown flag and leader updates
    condition_variable taskCondition;  // Used to notify threads that there are tasks in the queue
    bool shutdownFlag;                 // Flag to stop the threads if set to true
    int currentLeader;                 // Represents the leader worker thread

public:
    LFP(int numWorkers);                 // Constructor to initialize with the number of workers
    ~LFP();                              // Destructor
    void addTask(function<void()> task); // Add a task to the task queue
    void startProcessing();              // Start the task processing
    void stopProcessing();               // Stop the task processing
};

#endif // LFP_HPP