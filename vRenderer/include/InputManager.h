#pragma once

#include <GLFW/glfw3.h>
#include <vector>

#include "IMouseEventHandler.h"
#include <functional>

class InputManager
{
public:

    using OnMouseScrollCallback = std::function<void(float)>;

    static InputManager& instance()
    {
        return *_instance;
    }

    // Subscribe to scroll events for a specific window
    void subscribeToMouseScroll(const OnMouseScrollCallback& callback);

    static bool init(GLFWwindow* window);
    static bool isInitialized();

    void update();

private:

    GLFWwindow* window;
    static bool initialized;
    // Make constructor private to prevent direct instantiation

    InputManager();

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    // Delete copy constructor and assignment operator
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    static InputManager* _instance;

    std::vector<OnMouseScrollCallback> onScrollCalls;
};