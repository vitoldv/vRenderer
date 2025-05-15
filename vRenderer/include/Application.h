#pragma once

#include <iostream>
#include <string>

#include "OpenGLRenderer.h"
#include "VulkanRenderer.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "AppContext.h"

#include "Model.h"
#include "utils.h"
#include "editor_settings.h"

#include "OrbitCamera.h"
#include "FpvCamera.h"

//#define CURRENT_API API::VULKAN
#define CURRENT_API API::OPENGL

class Application
{
public:

	enum API
	{
		VULKAN,
		OPENGL
	};

	Application() = default;

	int run();

private:

	API api;
	GLFWwindow* window;
	IRenderer* renderer;
	AppContext* context;

	// Frame time control 
	int previousFrameTime = 0;
	int currentFrameTime = 0;

	// Scene
	std::string selectedModelName;
	bool newSelection;

	BaseCamera* camera;
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