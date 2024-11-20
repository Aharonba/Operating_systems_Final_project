#include "PAO.hpp"

/**
 * @brief Constructor for the PAO class, initializes the worker pool with task functions.
 *
 * @param taskFunctions A vector of task functions to be processed by the workers.
 * Each worker in the pool will be assigned one of the provided task functions.
 */
PAO::PAO(const std::vector<std::function<void(void *)>> &taskFunctions) : terminateFlag(false)
{
    // Initialize worker pool with task functions
    for (const auto &function : taskFunctions)
    {
        auto mutexPtr = std::make_unique<std::mutex>();
        auto conditionPtr = std::make_unique<std::condition_variable>();
        workerPool.push_back({nullptr, function, std::queue<void *>(), std::move(mutexPtr), std::move(conditionPtr), nullptr});
    }

    // Link each worker to the next worker's task queue (for task passing)
    for (size_t i = 0; i < workerPool.size() - 1; ++i)
    {
        workerPool[i].nextWorkerQueue = &workerPool[i + 1].taskQueue;
    }
}

/**
 * @brief Destructor for the PAO class, ensures worker threads are terminated and joined.
 */
PAO::~PAO()
{
    terminateWorkers();

    // Join all worker threads
    for (auto &worker : workerPool)
    {
        if (worker.workerThread && worker.workerThread->joinable())
        {
            worker.workerThread->join();
        }
    }
}

/**
 * @brief Enqueues a task to be processed by the first worker in the pool.
 *
 * @param taskData The data associated with the task to be processed by the worker.
 */
void PAO::enqueueTask(void *taskData)
{
    // Lock necessary mutexes to protect shared resources
    std::lock_guard<std::mutex> lock(workerPoolMutex);
    std::lock_guard<std::mutex> taskLock(taskDataMutex);
    std::lock_guard<std::mutex> queueLock(*(workerPool[0].queueLock));

    // Add task to the first worker's queue and notify the worker
    workerPool[0].taskQueue.push(taskData);
    workerPool[0].queueCondition->notify_one();
}

/**
 * @brief Initializes worker threads and starts the task execution pipeline.
 *
 * Worker threads are created and begin processing tasks from the queue.
 */
void PAO::initializeWorkers()
{
    // Set the terminate flag to false and initialize workers
    std::lock_guard<std::mutex> lock(terminateFlagMutex);
    terminateFlag = false;

    // Initialize and start each worker thread
    for (size_t i = 0; i < workerPool.size(); ++i)
    {
        Worker *subsequentWorker = (i + 1 < workerPool.size()) ? &workerPool[i + 1] : nullptr;
        workerPool[i].workerThread = std::make_unique<std::thread>(&PAO::executeWorkerTask, this, std::ref(workerPool[i]), subsequentWorker);
    }
}

/**
 * @brief Terminates all worker threads and signals them to stop processing.
 */
void PAO::terminateWorkers()
{
    // Set terminate flag to true to stop all workers
    {
        std::lock_guard<std::mutex> lock(terminateFlagMutex);
        terminateFlag = true;
    }

    // Notify all workers to stop processing tasks
    for (auto &worker : workerPool)
    {
        std::lock_guard<std::mutex> lock(*worker.queueLock);
        worker.queueCondition->notify_all();
    }
}

/**
 * @brief Executes a task for a worker and passes the task to the next worker if applicable.
 *
 * @param currentWorker The worker that is executing the current task.
 * @param subsequentWorker The next worker in the pipeline, or nullptr if this is the last worker.
 */
void PAO::executeWorkerTask(Worker &currentWorker, Worker *subsequentWorker)
{
    while (true)
    {
        bool terminate = false;
        
        // Check if terminate flag is set
        {
            std::lock_guard<std::mutex> lock(terminateFlagMutex);
            terminate = terminateFlag;
        }

        if (terminate) return; // Terminate the worker if the flag is set

        void *taskData = nullptr;
        
        // Wait for task data to become available in the worker's queue
        {
            std::unique_lock<std::mutex> lock(*currentWorker.queueLock);
            currentWorker.queueCondition->wait(lock, [&]() {
                std::lock_guard<std::mutex> lock(terminateFlagMutex);
                return terminateFlag || !currentWorker.taskQueue.empty();
            });

            if (terminateFlag) return; // Terminate if the flag is set

            taskData = currentWorker.taskQueue.front();
            currentWorker.taskQueue.pop();
        }

        // Execute the task function if available
        if (currentWorker.taskFunction)
        {
            std::lock_guard<std::mutex> lock(taskDataMutex);
            currentWorker.taskFunction(taskData);
        }

        // Pass the task to the next worker if available
        if (subsequentWorker)
        {
            std::lock_guard<std::mutex> lock(*subsequentWorker->queueLock);
            subsequentWorker->taskQueue.push(taskData);
            subsequentWorker->queueCondition->notify_one();
        }

        // Check if the worker should terminate after completing the task
        {
            std::lock_guard<std::mutex> lock(terminateFlagMutex);
            if (terminateFlag) return;
        }
    }
}
