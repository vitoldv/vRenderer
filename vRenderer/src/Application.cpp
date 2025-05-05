#include "Application.h"
#include "imgui/imgui_helper.h"

Application::Application()
{
	
}

void Application::initWindow(std::string title, const int width, const int height)
{
	glfwInit();
	// Set GLFW to NOT work with OpenGL
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}


int Application::initApplication()
{
	context = &AppContext::instance();

	InputManager::init(window);
	inputManager = &InputManager::instance();

	// Create and initialize Vulkan Renderer Instance
	if (vulkanRenderer.init(window) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}
	else
	{
		vulkanRenderer.setImguiCallback(std::bind(&Application::imguiMenu, this));
	}

	camera = Camera(FOV_ANGLES, Z_NEAR, Z_FAR, WINDOW_WIDTH, WINDOW_HEIGHT, true);
	inputManager->subscribeToMouseScroll(std::bind(&Camera::OnMouseScroll, &camera, std::placeholders::_1));
	vulkanRenderer.setCamera(&camera);

	modelToRender = new Model(1, MODEL_ASSETS("Statue\\Statue.obj"));
	vulkanRenderer.addToRendererTextured(*modelToRender);

	return 0;
}

void Application::processInput()
{
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	camera.OnMouseMove(xpos, ypos, glfwGetMouseButton(window, 0) == GLFW_PRESS);
}

void Application::update()
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

void Application::render()
{
	vulkanRenderer.draw();
}

void Application::cleanup()
{
	free(modelToRender);
	vulkanRenderer.cleanup();
}

void Application::destroyWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::imguiMenu()
{
	imgui_helper::ShowTransformEditor(position, rotation, scale);
}


int Application::run()
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

		context->deltaTime = frameTime / 1000.0f;
		previousFrameTime = currentFrameTime;

		processInput();
		update();
		render();
	}

	cleanup();

	destroyWindow();
}