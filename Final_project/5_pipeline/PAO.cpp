#include "PAO.hpp"

/**
 * Construct a new PAO object.
 * Takes functions to be executed by the workers.
 */
PAO::PAO(const std::vector<std::function<void(void *)>> &taskFunctions) : terminateFlag(false)
{
    // Initialize workerPool with Worker structs
    for (const auto &function : taskFunctions)
    {
        std::mutex *mutexPtr = new std::mutex();
        std::condition_variable *conditionPtr = new std::condition_variable();
        workerPool.push_back({nullptr, function, std::queue<void *>(), mutexPtr, conditionPtr, nullptr});
    }
    // Link each worker's task queue to the next one in the chain
    for (size_t i = 0; i < workerPool.size() - 1; ++i)
    {
        workerPool[i].nextWorkerQueue = &workerPool[i + 1].taskQueue;
    }
}

PAO::~PAO()
{

    terminateWorkers(); // Stop all threads

    // Clean up all worker resources
    for (auto &worker : workerPool)
    {
        if (worker.workerThread && worker.workerThread->joinable())
        {
            worker.workerThread->join();
        }

        delete worker.workerThread;
        delete worker.queueLock;
        delete worker.queueCondition;
    }
}

/**
 * Adds a task to the first worker's task queue.
 */
void PAO::enqueueTask(void *taskData)
{
    std::lock_guard<std::mutex> lock(*(workerPool[0].queueLock));
    workerPool[0].taskQueue.push(taskData);
    workerPool[0].queueCondition->notify_one(); // Notify the first worker
}

/**
 * Initializes all worker threads.
 * Each worker thread starts with executeWorkerTask.
 */
void PAO::initializeWorkers()
{
    terminateFlag = false;

    for (size_t i = 0; i < workerPool.size(); ++i)
    {
        Worker *subsequentWorker = (i + 1 < workerPool.size()) ? &workerPool[i + 1] : nullptr;
        workerPool[i].workerThread = new std::thread(&PAO::executeWorkerTask, this, std::ref(workerPool[i]), subsequentWorker);
    }
}

void PAO::terminateWorkers()
{
    terminateFlag = true;
    for (auto &worker : workerPool)
    { // Notify all workers to stop
        std::lock_guard<std::mutex> lock(*worker.queueLock);
        worker.queueCondition->notify_all();
    }
}

/**
 * Wraps the worker's execution function.
 * Repeats tasks until terminateFlag is set to true.
 */
void PAO::executeWorkerTask(Worker &currentWorker, Worker *subsequentWorker)
{
    while (!terminateFlag)
    {

        // Retrieve a task:
        void *taskData = nullptr;
        {
            std::unique_lock<std::mutex> lock(*currentWorker.queueLock);
            currentWorker.queueCondition->wait(lock, [&]()
                                               { return terminateFlag || !currentWorker.taskQueue.empty(); });

            if (terminateFlag && currentWorker.taskQueue.empty())
                return;

            taskData = currentWorker.taskQueue.front();
            currentWorker.taskQueue.pop();
        }

        // Execute the task
        if (currentWorker.taskFunction)
        {
            currentWorker.taskFunction(taskData);
        }

        // Pass task to the next worker if applicable
        if (subsequentWorker)
        {
            std::lock_guard<std::mutex> lock(*subsequentWorker->queueLock);
            subsequentWorker->taskQueue.push(taskData);

            subsequentWorker->queueCondition->notify_one();
        }
        if (terminateFlag)
            return;
    }
}
