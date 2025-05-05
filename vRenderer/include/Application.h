#pragma once

#include <iostream>
#include <string>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "AppContext.h"
#include "InputManager.h"
#include "VulkanRenderer.h"
#include "Model.h"
#include "utils.h"

class Application
{
public:

	Application();

	int run();

private:

	GLFWwindow* window;
	VulkanRenderer vulkanRenderer;
	AppContext* context;
	InputManager* inputManager;

	// Frame time control 
	int previousFrameTime = 0;
	int currentFrameTime = 0;

	// Scene
	Camera camera;
	Model* modelToRender;
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

	void initWindow(std::string title, const int width, const int height);
	int initApplication();
	void processInput();
	void update();
	void render();
	void cleanup();
	void destroyWindow();
	void imguiMenu();
};