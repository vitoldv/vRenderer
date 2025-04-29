#include <iostream>
#include <string>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "utils.h"
#include "Model.h"

#define WINDOW_TITLE		"vRenderer"
#define WINDOW_WIDTH		1920
#define WINDOW_HEIGHT		1080

#define FPS_LIMIT			false
#define TARGET_FPS			60
#define TARGET_FRAME_TIME	(1000 / TARGET_FPS)

#define ASSETS_FOLDER "vRenderer\\assets\\"
#define MODEL_ASSETS(asset) concat(concat(ASSETS_FOLDER, "models\\"), asset)

GLFWwindow* window;

// Frame time control 
int previousFrameTime = 0;
int currentFrameTime = 0;
float deltaTime = 0;

Model* modelToRender;

void initWindow(std::string title, const int width, const int height)
{
	glfwInit();
	// Set GLFW to NOT work with OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

void initApplication()
{
	modelToRender = new Model(1, MODEL_ASSETS("SeahawkBlender\\SeahawkBlender.obj"));
}

void cleanup()
{
	free(modelToRender);
}

void destroyWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void processInput()
{
}

void update()
{
}

void render()
{
}

int main()
{
	initWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
	initApplication();

	float frameTime = 0;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		currentFrameTime = glfwGetTime() * 1000.0f;
		frameTime = currentFrameTime - previousFrameTime;

		if (FPS_LIMIT && frameTime < TARGET_FRAME_TIME) continue;

		deltaTime = frameTime / 1000.0f;
		previousFrameTime = currentFrameTime;

		processInput();
		update();
		render();
	}

	cleanup();

	destroyWindow();
}