#include "ThreadDispatcher.h"

// Static member definitions
std::mutex MainThreadDispatcher::queueMutex;
std::queue<Task> MainThreadDispatcher::taskQueue;
std::condition_variable MainThreadDispatcher::queueCondition;

void MainThreadDispatcher::process() {
    std::queue<Task> tasks;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        std::swap(tasks, taskQueue);
    }

    // Execute each task in the queue
    while (!tasks.empty()) {
        Task task = tasks.front();
        tasks.pop();
        task.function(task.data); // Call the function with its data
    }
}

// Add a task to the dispatcher (thread-safe)
void MainThreadDispatcher::post(void (*function)(void*), void* data) {
    std::lock_guard<std::mutex> lock(queueMutex);
    taskQueue.push({ function, data });
    queueCondition.notify_one();
}