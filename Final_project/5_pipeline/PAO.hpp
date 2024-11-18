#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <string>
#include <utility>

class PAO
{
private:
    // Worker struct that represents a worker thread
    struct Worker
    {
        std::unique_ptr<std::thread> workerThread;               // Change to unique_ptr
        std::function<void(void *)> taskFunction;                // The function that the worker will execute
        std::queue<void *> taskQueue;                            // Queue of tasks for the worker
        std::unique_ptr<std::mutex> queueLock;                   // Mutex for the task queue
        std::unique_ptr<std::condition_variable> queueCondition; // Condition variable for worker synchronization
        std::queue<void *> *nextWorkerQueue;                     // Pointer to the next worker's task queue
    };

    void executeWorkerTask(Worker &currentWorker, Worker *subsequentWorker);

    std::vector<Worker> workerPool;
    std::atomic<bool> terminateFlag; // Atomic flag to signal thread termination

public:
    PAO(const std::vector<std::function<void(void *)>> &taskFunctions); // Constructor receives functions for workers to execute
    ~PAO();
    PAO() = default;

    void enqueueTask(void *taskData);
    void initializeWorkers();
    void terminateWorkers();
};