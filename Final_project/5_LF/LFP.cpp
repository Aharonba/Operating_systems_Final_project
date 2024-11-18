#include "LFP.hpp"

// Constructor to initialize the worker pool
LFP::LFP(int numWorkers) : shutdownFlag(false), currentLeader(0)
{
    for (int i = 0; i < numWorkers; ++i)
    {
        workerThreads.emplace_back(&LFP::taskProcessor, this, i); // Create threads and add to the workerThreads vector
    }
}

// Destructor: Ensure to stop processing when the object goes out of scope
LFP::~LFP()
{
    stopProcessing();
}

// Adds a new task to the task queue
void LFP::addTask(function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex); // Lock the mutex to ensure thread-safe access
        taskQueue.push(task);                         // Add the task to the queue
        taskCondition.notify_all();                   // Notify one worker that there is a new task
    }
}

// Stops the task processing and shuts down workers
void LFP::stopProcessing()
{
    // Notify workers to stop processing
    {
        std::lock_guard<std::mutex> lock(shutdownMutex);
        shutdownFlag = true; // Signal shutdown to workers
    }

    // Notify all worker threads to stop
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskCondition.notify_all(); // Wake up all threads
    }

    // Join all worker threads
    for (std::thread &t : workerThreads)
    {
        if (t.joinable())
        {
            t.join(); // Wait for each worker thread to finish
        }
    }
}

// Worker function to process tasks in the queue
void LFP::taskProcessor(int workerId)
{
    while (true)
    {
        std::function<void()> currentTask;

        {
            std::unique_lock<std::mutex> lock(queueMutex); // Lock the queue

            // Wait until there are tasks in the queue or shutdown is requested
            taskCondition.wait(lock, [this]()
                               {
                std::lock_guard<std::mutex> stopLock(shutdownMutex); // Lock shutdown mutex separately
                return shutdownFlag || !taskQueue.empty(); });

            // Check if we should exit the thread
            {
                std::lock_guard<std::mutex> stopLock(shutdownMutex);
                if (shutdownFlag && taskQueue.empty())
                    return; // Exit the thread
            }

            // If taskQueue is not empty, assign the task
            if (!taskQueue.empty() && this->currentLeader == workerId)
            {
                currentTask = taskQueue.front(); // Get the task
                taskQueue.pop();                 // Remove it from the queue
            }
            else
            {
                continue; // Skip if this worker is not the leader
            }
        }

        // Update the leader
        {
            std::lock_guard<std::mutex> lock(shutdownMutex);
            currentLeader = (currentLeader + 1) % workerThreads.size(); // Set the next leader
        }

        // Execute the task outside of any locks
        currentTask();
    }
}
