#include "Application.h"
#include "imgui/imgui_helper.h"

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

	// Create and initialize Vulkan Renderer Instance
	if (vulkanRenderer.init(window) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}
	else
	{
		vulkanRenderer.setImguiCallback(std::bind(&Application::imguiMenu, this));
	}   

	initInput(window);

	// Camera initilization
	camera = new OrbitCamera(FOV_ANGLES, Z_NEAR, Z_FAR, WINDOW_WIDTH, WINDOW_HEIGHT, true);
	EventBinder::Bind(&BaseCamera::onMouseMove, camera, inp::onMouseMove);
	EventBinder::Bind(&BaseCamera::onMouseScroll, camera, inp::onMouseScroll);
	EventBinder::Bind(&BaseCamera::onKey, camera, inp::onKey);

	vulkanRenderer.setCamera(camera);

	return 0;
}

void Application::processInput()
{
	process(window);
}

void Application::update()
{
	if (this->newSelection)
	{
		if (modelToRender != nullptr)
		{
			vulkanRenderer.removeFromRenderer(modelToRender->id);
			delete modelToRender;
			modelToRender = nullptr;
		}
		
		std::string model = this->selectedModelName + "\\" + this->selectedModelName + ".obj";
		modelToRender = new Model(1, MODEL_ASSETS(model.c_str()));
		vulkanRenderer.addToRendererTextured(*modelToRender);

		newSelection = false;
	}

	if (modelToRender != nullptr)
	{
		glm::mat4 t = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 rx = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0, 0));
		glm::mat4 ry = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0, 1.0f, 0));
		glm::mat4 rz = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0, 0, 1.0f));
		glm::mat4 s = glm::scale(glm::mat4(1.0f), scale);
		glm::mat4 transform = t * rz * ry * rx * s;
		vulkanRenderer.updateModelTransform(modelToRender->id, transform);
	}

	camera->update();
}

void Application::render()
{
	vulkanRenderer.draw();
}

void Application::cleanup()
{
	delete camera;
	camera = nullptr;

	delete modelToRender;
	modelToRender = nullptr;

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
	imgui_helper::DrawAssetBrowser(MODEL_ASSETS_FOLDER, c_supportedFormats, selectedModelName, this->newSelection);
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