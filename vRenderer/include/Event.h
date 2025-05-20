#pragma once

#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>

#include <iostream>

struct CallData
{
    void* classType;
    void* event;

    bool operator==(const CallData& other) const {
        return classType == other.classType &&
            event == other.event;
    }
};

struct CallDataHash {
    size_t operator()(const CallData& cd) const {
        return (std::hash<void*>()(cd.classType)) ^
            (std::hash<void*>()(cd.event) << 1);
    }
};

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
        for (auto& pair : callbacks)
        {
            pair.second(args...);
        }
    }

    void add(callback_t callback, CallData callData)
    {
        callbacks.emplace(callData, callback);
    }

    void remove(CallData callData)
    {      
        auto it = callbacks.find(callData);
        if (it != callbacks.end())
        {
            callbacks.erase(it);
        }     
    }

private:
    std::unordered_map<CallData, callback_t, CallDataHash> callbacks;
};


static class EventBinder
{

public:

    template <typename Callable, typename ClassType, typename... Params>
    static void Bind(Callable callable, ClassType* classType, Event<Params...>& event)
    {
        CallData callData = { 
            static_cast<void*>(classType),
            static_cast<void*>(&event)
        };

        // If an event with more parameters is used add a new conditional here.
        if constexpr (sizeof...(Params) == 0)
        {
            event.add(std::bind(callable, classType), callData);
        }
        else if constexpr (sizeof...(Params) == 1)
        {
            event.add(std::bind(callable, classType, std::placeholders::_1), callData);
        }
        else if constexpr (sizeof...(Params) == 2)
        {
            event.add(std::bind(callable, classType, std::placeholders::_1, std::placeholders::_2), callData);
        }
        else if constexpr (sizeof...(Params) == 3)
        {
            event.add(std::bind(callable, classType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), callData);
        }
        else if constexpr (sizeof...(Params) == 4)
        {
            event.add(std::bind(callable, classType, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), callData);
        }
    }

    template <typename Callable, typename ClassType, typename... Params>
    static void Unbind(Callable callable, ClassType* classType, Event<Params...>& event)
    {
        CallData callData = {
            static_cast<void*>(classType),
            static_cast<void*>(&event)
        };

        event.remove(callData);
    }
};