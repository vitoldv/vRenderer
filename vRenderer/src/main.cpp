#define GLFW_INCLUDE_VULKAN
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <string>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "utils.h"
#include "Model.h"

#include "VulkanRenderer.h"
#include "imgui/imgui_helper.h"

#define WINDOW_TITLE		"vRenderer"
#define WINDOW_WIDTH		1920
#define WINDOW_HEIGHT		1080

#define FPS_LIMIT			false
#define TARGET_FPS			60
#define TARGET_FRAME_TIME	(1000 / TARGET_FPS)

#define ASSETS_FOLDER "vRenderer\\assets\\"
#define MODEL_ASSETS(asset) concat(concat(ASSETS_FOLDER, "models\\"), asset)

GLFWwindow* window;
VulkanRenderer vulkanRenderer;

// Frame time control 
int previousFrameTime = 0;
int currentFrameTime = 0;
float deltaTime = 0;

Camera camera;

Model* modelToRender;
glm::vec3 position(0.0f, 0.0f, 0.0f);
glm::vec3 rotation(0.0f, 0.0f, 0.0f);
glm::vec3 scale(1.0f, 1.0f, 1.0f);

void imguiMenu()
{
	imgui_helper::ShowTransformEditor(position, rotation, scale);
	//imgui_helper::ShowCameraEditor(actualCameraPosition, cameraDistance);
	ImGui::Begin("Camera rot speed");
	//ImGui::DragFloat("SPEED", &cameraRotationSpeed, 1.0f, 1.0f, 1000.0f);
	//imgui_helper::ShowAdvancedVec3Editor("camera rotation vec", cameraRotation, sync);
	ImGui::End();
}

void initWindow(std::string title, const int width, const int height)
{
	glfwInit();
	// Set GLFW to NOT work with OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);	

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.OnMouseScroll(yoffset);
}

int initApplication()
{
	// Create and initialize Vulkan Renderer Instance
	if (vulkanRenderer.init(window) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}
	else
	{
		vulkanRenderer.setImguiCallback(imguiMenu);
	}

	camera = Camera(FOV_ANGLES, Z_NEAR, Z_FAR, WINDOW_WIDTH, WINDOW_HEIGHT, true);
	glfwSetScrollCallback(window, scroll_callback);

	vulkanRenderer.setCamera(&camera);

	modelToRender = new Model(1, MODEL_ASSETS("Statue\\Statue.obj"));
	vulkanRenderer.addToRendererTextured(*modelToRender);

	return 0;
}

void processInput()
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	camera.OnMouseMove(xpos, ypos, glfwGetMouseButton(window, 0) == GLFW_PRESS);
}

void update()
{
	glm::mat4 t = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 rx = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0, 0));
	glm::mat4 ry = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0, 1.0f, 0));
	glm::mat4 rz = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0, 0, 1.0f));
	glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 transform = t * rz * ry * rx * s;
	vulkanRenderer.updateModelTransform(modelToRender->id, transform);

	camera.update();
}

void render()
{
	vulkanRenderer.draw();
}

void cleanup()
{
	free(modelToRender);
	vulkanRenderer.cleanup();
}

void destroyWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

int main()
{
	initWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);

	if (initApplication() == EXIT_FAILURE)
		return EXIT_FAILURE;

	float frameTime = 0;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		currentFrameTime = glfwGetTime() * 1000.0f;
		frameTime = currentFrameTime - previousFrameTime;

		if (FPS_LIMIT && frameTime < TARGET_FRAME_TIME) continue;

		deltaTime = frameTime / 1000.0f;
		camera.deltaTime = deltaTime;
		previousFrameTime = currentFrameTime;

		processInput();
		update();
		render();
	}

	cleanup();

	destroyWindow();
}