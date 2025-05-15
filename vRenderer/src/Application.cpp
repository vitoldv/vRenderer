//--- do not remove, it's here for the God's sake
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//-------------------------------------------------

#include "Application.h"
#include "imgui/imgui_helper.h"

void Application::initWindow(std::string title, const int width, const int height)
{
	glfwInit();
	if (api == VULKAN)
	{
		// Set GLFW to NOT work with OpenGL
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	}
	else if (api == OPENGL)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

	if (api == OPENGL)
	{
		glfwMakeContextCurrent(window);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{		
			throw std::runtime_error("Failed to initialize GLAD");
		}
	}
}

int Application::initApplication()
{
	context = &AppContext::instance();

	if (api == VULKAN)
	{
		renderer = new VulkanRenderer();
	}
	else if (api == OPENGL)
	{
		renderer = new OpenGLRenderer();
	}

	// Create and initialize Renderer Instance
	if (renderer->init(window) == EXIT_FAILURE)
	{
		return EXIT_FAILURE;
	}
	else
	{
		renderer->setImguiCallback(std::bind(&Application::imguiMenu, this));
	}   

	initInput(window);

	// Camera initilization
	camera = new OrbitCamera(FOV_ANGLES, Z_NEAR, Z_FAR, WINDOW_WIDTH, WINDOW_HEIGHT, api == API::VULKAN);
	EventBinder::Bind(&BaseCamera::onMouseMove, camera, inp::onMouseMove);
	EventBinder::Bind(&BaseCamera::onMouseScroll, camera, inp::onMouseScroll);
	EventBinder::Bind(&BaseCamera::onKey, camera, inp::onKey);

	renderer->setCamera(camera);

	return 0;
}

void Application::processInput()
{
	IsUIFocused = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
	process(window);
}

void Application::update()
{
	if (this->newSelection)
	{
		if (modelToRender != nullptr)
		{
			renderer->removeFromRenderer(modelToRender->id);
			delete modelToRender;
			modelToRender = nullptr;
		}
		
		std::string model = this->selectedModelName + "\\" + this->selectedModelName + ".obj";
		modelToRender = new Model(1, MODEL_ASSETS(model.c_str()));
		renderer->addToRendererTextured(*modelToRender);

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
		renderer->updateModelTransform(modelToRender->id, transform);
	}

	camera->update();
}

void Application::render()
{
	renderer->draw();
}

void Application::cleanup()
{
	delete camera;
	camera = nullptr;

	delete modelToRender;
	modelToRender = nullptr;

	renderer->cleanup();
}

void Application::destroyWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::imguiMenu()
{	
	// SETTINGS EDITOR WINDOW
	{
		ImGui::Begin("vRenderer Settings", nullptr, ImGuiWindowFlags_None);
		if (ImGui::BeginTabBar("Menus")) {
			if (ImGui::BeginTabItem("Asset Browser")) {
				imgui_helper::DrawAssetBrowser(MODEL_ASSETS_FOLDER, c_supportedFormats, selectedModelName, this->newSelection);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Transform Editor")) {
				imgui_helper::ShowTransformEditor(position, rotation, scale); // Renamed!
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
	}

	imgui_helper::DrawFPSOverlay(api == API::VULKAN ? "Vulkan" : "OpenGL");
}

int Application::run()
{
	this->api = CURRENT_API;

	initWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);

	if (initApplication() == EXIT_FAILURE)
		return EXIT_FAILURE;

	float frameTime = 0;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		if (api == OPENGL)
			glfwSwapBuffers(window);

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