#include "LFP.hpp"

// Constructor to initialize the worker pool
LFP::LFP(int numWorkers) : shutdownFlag(false), currentLeader(0)
{
    for (int i = 0; i < numWorkers; ++i)
    {
        workerThreads.emplace_back(&LFP::taskProcessor, this, i); // Create threads and add to the workerThreads vector
        workerIDs.push_back(i);                                   // Store worker IDs
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
    }
    taskCondition.notify_one(); // Notify one worker that there is a new task
}

// Starts the task processing by workers
void LFP::startProcessing()
{
    std::lock_guard<std::mutex> lock(shutdownMutex); // Lock the shutdown mutex
    shutdownFlag = false;                            // Set the shutdown flag to false, enabling task processing
}

// Stops the task processing and shuts down workers
void LFP::stopProcessing()
{
    {
        std::lock_guard<std::mutex> lock(shutdownMutex); // Lock the shutdown mutex
        shutdownFlag = true;                             // Set the shutdown flag to true, signaling workers to stop
    }

    std::lock_guard<std::mutex> lock(queueMutex); // Lock on queueMutex for thread-safe notification
    taskCondition.notify_all();                   // Notify all workers to stop or process the remaining tasks

    // Join all threads (wait for them to finish processing)
    for (thread &t : workerThreads)
    {
        if (t.joinable())
        {
            t.join(); // Join each worker thread
        }
    }
}

// Worker function to process tasks in the queue
void LFP::taskProcessor(int workerId)
{
    while (true)
    {
        std::function<void()> currentTask; // Task to be executed

        {
            std::unique_lock<std::mutex> lock(queueMutex); // Lock the queue mutex to access the task queue

            // Wait until there are tasks in the queue or shutdown is requested
            taskCondition.wait(lock, [this]()
                               {
                std::lock_guard<std::mutex> stopLock(shutdownMutex); // Lock shutdown mutex separately
                return shutdownFlag || !taskQueue.empty(); });

            // If shutdown is set and the queue is empty, exit
            {
                std::lock_guard<std::mutex> stopLock(shutdownMutex); // Lock shutdown mutex separately
                if (shutdownFlag && taskQueue.empty())
                    return;
            }

            // If the current worker is the leader, assign and remove the task from the queue
            if (!taskQueue.empty() && this->currentLeader == workerId)
            {
                currentTask = taskQueue.front(); // Get the task from the front of the queue
                taskQueue.pop();                 // Remove the task from the queue
            }
            else
            {
                continue; // Skip if the current worker is not the leader
            }
        }

        // Update the leader after the task is assigned
        {
            std::lock_guard<std::mutex> lock(shutdownMutex);            // Lock shutdown mutex to update the leader
            currentLeader = (currentLeader + 1) % workerThreads.size(); // Set the next leader
        }

        // Execute the task outside of any locks
        currentTask();
    }
}
