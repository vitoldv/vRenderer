#include "InputManager.h"

#include <iostream>

InputManager* InputManager::_instance = nullptr;
bool InputManager::initialized = false;

InputManager::InputManager()
{

}

void InputManager::subscribeToMouseScroll(const OnMouseScrollCallback& callback)
{
	if (!initialized) return;

	onScrollCalls.push_back(callback);
}

bool InputManager::init(GLFWwindow* window)
{
	if (!initialized && window != nullptr)
	{
		_instance = new InputManager();

		_instance->window = window;
	
		glfwSetScrollCallback(_instance->window, scroll_callback);

		initialized = true;
	}

	return initialized;
}

bool InputManager::isInitialized()
{
	return initialized;
}

void InputManager::update()
{

}


void InputManager::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	for (int i = 0; i < _instance->onScrollCalls.size(); i++)
	{
		_instance->onScrollCalls[i](yoffset);
	}
}
