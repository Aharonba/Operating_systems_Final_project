#include "LFP.hpp"

using namespace std;

// Constructor to initialize the worker pool
LFP::LFP(int numWorkers) : shutdownFlag(false), currentLeader(0)
{
    for (int i = 0; i < numWorkers; ++i)
    {                                                             // Create threads and add them to the workerThreads vector
        workerThreads.emplace_back(&LFP::taskProcessor, this, i); // Pass the worker ID to the taskProcessor
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
        lock_guard<mutex> lock(queueMutex); // Lock the mutex to ensure thread-safe access
        taskQueue.push(task);               // Add the task to the queue
        taskCondition.notify_one();         // Notify one worker that there is a new task
    }
}

// Starts the task processing by workers
void LFP::startProcessing()
{
    {
        lock_guard<mutex> lock(shutdownMutex); // Lock the shutdown mutex
        shutdownFlag = false;                  // Set the shutdown flag to false, enabling task processing
    }
}

// Stops the task processing and shuts down workers
void LFP::stopProcessing()
{
    {
        lock_guard<mutex> lock(shutdownMutex); // Lock the shutdown mutex
        shutdownFlag = true;                   // Set the shutdown flag to true, signaling workers to stop
    }

    {
        lock_guard<mutex> lock(queueMutex); // Lock the queue mutex to safely notify workers
        taskCondition.notify_all();         // Notify all workers to stop or process the remaining tasks
    }

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
    {                                 // Keep processing tasks until the shutdown flag is set
        function<void()> currentTask; // Task to be executed

        {
            unique_lock<mutex> lock(queueMutex); // Lock the queue mutex to access the task queue
            taskCondition.wait(lock, [this]()
                               {
                                   lock_guard<mutex> stopLock(shutdownMutex); // Lock the shutdown mutex
                                   return shutdownFlag || !taskQueue.empty(); // Wait until there are tasks or shutdown is requested
                               });

            // Check if the worker should stop processing
            {
                lock_guard<mutex> stopLock(shutdownMutex); // Lock shutdown mutex to check shutdownFlag
                if (shutdownFlag && taskQueue.empty())
                    return; // Exit if shutdown is true and no tasks remain
            }

            // If there are tasks, and the current worker is the leader, assign and remove the task from the queue
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
            lock_guard<mutex> lock(queueMutex); // Lock the queue mutex to update the leader
            std::cout << currentLeader << endl;
            currentLeader = (size_t)(currentLeader + 1) % workerThreads.size(); // Set the next leader
        }

        currentTask(); // Execute the task
    }
}
