#include "ThreadDispatcher.h"

ThreadDispatcher::ThreadDispatcher()
{
    // Allocate the number of threads equal to the number of logical CPU cores
    workerPool = std::make_unique<ThreadPool>(std::thread::hardware_concurrency());
}

void ThreadDispatcher::process()
{
    std::queue<Task> tasks;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        std::swap(tasks, taskQueue);
    }

    while (!tasks.empty()) 
    {
        Task task = tasks.front();
        tasks.pop();
        task.function(task.data);
    }
}

// Add a task to the main thread dispatcher
void ThreadDispatcher::dispatch_main_internal(void (*function)(void*), void* data)
{
    std::lock_guard<std::mutex> lock(queueMutex);
    taskQueue.push({ function, data });
}