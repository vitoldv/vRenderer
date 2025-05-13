#pragma once

#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <functional>

/// <summary>
/// Simple Event template class. Use EventBinder::Bind to add any function/method call to event callbacks.
/// Example: EventBinder::Bind(&Class::method, obj, event)
/// </summary>
/// <typeparam name="...Params"></typeparam>
template <typename... Params>
class Event
{
    using callback_t = std::function<void(Params...)>;

public:
     
    Event() = default;
    ~Event() = default;

    void invoke(Params... args)
    {
        for (auto& callback : callbacks)
        {
            callback(args...);
        }
    }

    void add(callback_t callback)
    {
        callbacks.push_back(callback);
    }

    void remove(callback_t callback)
    {
        callbacks.erase(std::find(callbacks.begin(), callbacks.end(), callback));
    }

private:
    std::vector<callback_t> callbacks;
};


static class EventBinder
{

public:

    template <typename Callable, typename ClassType, typename... Params>
    static void Bind(Callable callable, ClassType* classType, Event<Params...>& event)
    {
        // If an event with more parameters is used add a new conditional here.
        if constexpr (sizeof...(Params) == 0)
        {
            event.add(std::bind(callable, classType));
        }
        else if constexpr (sizeof...(Params) == 1)
        {
            event.add(std::bind(callable, classType, std::placeholders::_1));
        }
        else if constexpr (sizeof...(Params) == 2)
        {
            event.add(std::bind(callable, classType, std::placeholders::_1, std::placeholders::_2));
        }
        else if constexpr (sizeof...(Params) == 3)
        {
            event.add(std::bind(callable, classType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        }
        else if constexpr (sizeof...(Params) == 4)
        {
            event.add(std::bind(callable, classType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        }
    }
};