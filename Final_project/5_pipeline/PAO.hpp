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

/**
 * @class PAO
 * @brief Manages a pool of worker threads processing tasks in parallel.
 *
 * The PAO class uses a Producer-Action-Outcome pipeline where tasks are processed by a pool of workers.
 */
class PAO
{
private:
    /**
     * @struct Worker
     * @brief Represents a worker thread and its task queue.
     */
    struct Worker
    {
        std::unique_ptr<std::thread> workerThread;               ///< The worker thread
        std::function<void(void *)> taskFunction;                ///< The function that processes tasks
        std::queue<void *> taskQueue;                            ///< The queue of tasks assigned to the worker
        std::unique_ptr<std::mutex> queueLock;                   ///< Mutex to protect task queue access
        std::unique_ptr<std::condition_variable> queueCondition; ///< Condition variable for task availability
        std::queue<void *> *nextWorkerQueue;                     ///< Pointer to the next worker's task queue
    };

    std::mutex workerPoolMutex;            ///< Mutex for protecting worker pool
    std::mutex terminateFlagMutex;        ///< Mutex for the termination flag
    std::mutex taskDataMutex;             ///< Mutex for task data access

    /**
     * @brief Executes a task and passes it to the next worker.
     */
    void executeWorkerTask(Worker &currentWorker, Worker *subsequentWorker);

    std::vector<Worker> workerPool;        ///< Pool of workers
    std::atomic<bool> terminateFlag;       ///< Flag to signal worker termination

public:
    /**
     * @brief Initializes the PAO with task functions.
     *
     * @param taskFunctions List of task functions to be executed by workers.
     */
    PAO(const std::vector<std::function<void(void *)>> &taskFunctions);

    /**
     * @brief Cleans up worker resources.
     */
    ~PAO();

    /**
     * @brief Enqueues a task to be processed by the worker pool.
     */
    void enqueueTask(void *taskData);

    /**
     * @brief Starts the worker threads and begins task processing.
     */
    void initializeWorkers();

    /**
     * @brief Stops worker threads and terminates task processing.
     */
    void terminateWorkers();
};
