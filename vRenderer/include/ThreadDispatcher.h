#pragma once

#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>

#include "Singleton.h"
#include "ThreadPool.h"

struct Task
{
    void (*function)(void*);  // Raw function pointer
    void* data;               // Pointer to data for the function (if needed)
};

/// <summary>
/// Thread dispatcher for both main and worker threads
/// </summary>
class ThreadDispatcher : public Singleton<ThreadDispatcher>
{
    friend class Singleton<ThreadDispatcher>;

public:

    ThreadDispatcher();

    void process();

    template <typename Callable, typename... Args>
    void main(Callable callable, Args... args);
    template <typename Callable, typename... Args>
    void worker(Callable callable, Args... args);

private:

    std::mutex queueMutex;    
    std::queue<Task> taskQueue;   
    std::unique_ptr<ThreadPool> workerPool;

    void dispatch_main_internal(void (*function)(void*), void* data = nullptr);
};

/// <summary>
/// Dispatches provided callable to main thread
/// </summary>
template <typename Callable, typename... Args>
void ThreadDispatcher::main(Callable callable, Args... args) 
{
    struct Wrapper
    {
        Callable callable;
        std::tuple<Args...> args;

        // The function to invoke the callable
        static void invoke(void* data) {
            auto wrapper = static_cast<Wrapper*>(data);
            std::apply(wrapper->callable, wrapper->args);
            delete wrapper; // Clean up the wrapper
        }
    };

    // Create a wrapper to hold the callable and arguments
    auto* wrapper = new Wrapper{ callable, std::make_tuple(args...) };
    dispatch_main_internal(&Wrapper::invoke, wrapper);
}

/// <summary>
/// Dispatches provided callable to the available working thread
/// </summary>
template <typename Callable, typename... Args>
void ThreadDispatcher::worker(Callable callable, Args... args)
{
    workerPool->enqueue(callable, args...);
}
