#include <iostream>
#include <string>

#include <GLFW/glfw3.h>

#define WINDOW_TITLE		"vRenderer"
#define WINDOW_WIDTH		1920
#define WINDOW_HEIGHT		1080

GLFWwindow* window;

void initWindow(std::string title, const int width, const int height)
{
	glfwInit();
	// Set GLFW to NOT work with OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

int main()
{
	initWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	// Destroy window
	glfwDestroyWindow(window);
	glfwTerminate();
}