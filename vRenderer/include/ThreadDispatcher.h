//#pragma once
//
//#include <queue>
//#include <thread>
//#include <future>
//#include <mutex>
//
//class ThreadDispatcher
//{
//
//public:
//
//	enum class Thread
//	{
//		MAIN,
//	};
//
//	ThreadDispatcher()
//	{
//		
//	}
//
//	void process()
//	{
//		while (!handlers.empty())
//		{
//			auto& handler = handlers.front();
//			handler.function(handler.data);
//			handlers.pop();
//		}
//	}
//
//	template<typename Callback, typename... Args>
//	static void dispatch(Thread threadType, Callback callback);
//
//private:
//
//	// Struct to represent a task using a function pointer
//	struct Task
//	{
//		void (*function)(void*);  // Raw function pointer
//		void* data;               // Pointer to data for the function (if needed)
//	};
//
//	static std::queue<Task> handlers;
//	static std::mutex mutex;
//};
//
//template<typename Callback, typename... Args>
//static void ThreadDispatcher::dispatch(Thread threadType, Callback callback)
//{
//	std::lock_guard<std::mutex> lock(mutex);
//	struct Wrapper
//	{
//		Callback callable;
//		std::tuple<Args...> args;
//		// The function to invoke the callable
//		static void invoke(void* data)
//		{
//			auto wrapper = static_cast<Wrapper*>(data);
//			std::apply(wrapper->callable, wrapper->args);
//			delete wrapper; // Clean up the wrapper
//		}
//	};
//
//	auto* wrapper = new Wrapper{ callback, std::make_tuple(args...) };
//
//	handlers.push({&Wrapper::invoke, wrapper});
//}

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

// Struct to represent a task using a function pointer
struct Task {
    void (*function)(void*);  // Raw function pointer
    void* data;               // Pointer to data for the function (if needed)
};

// Simple MainThreadDispatcher
class MainThreadDispatcher 
{
public:

    // Add a task to the dispatcher (thread-safe)
    static void post(void (*function)(void*), void* data = nullptr);

    // Execute all tasks in the queue (called on the main thread)
    static void process();

    template <typename Callable, typename... Args>
    static void postWithArgs(Callable callable, Args... args);

private:
    static std::mutex queueMutex;        // Protect access to the task queue
    static std::queue<Task> taskQueue;   // Queue of tasks to execute
    static std::condition_variable queueCondition; // Optional for advanced use (e.g., waiting mechanics)
};

template <typename Callable, typename... Args>
void MainThreadDispatcher::postWithArgs(Callable callable, Args... args) {
    struct Wrapper {
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
    post(&Wrapper::invoke, wrapper);
}