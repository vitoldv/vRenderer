//--- do not remove, it's here for the God's sake
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//-------------------------------------------------

#include "Application.h"
#include "imgui/imgui_helper.h"

void Application::initWindow(std::string title, const int width, const int height)
{
	glfwInit();
	if (currentApi == RenderSettings::API::VULKAN)
	{
		// Set GLFW to NOT work with OpenGL
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	}
	else if (currentApi == RenderSettings::API::OPENGL)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

	if (currentApi == RenderSettings::API::OPENGL)
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

	if (currentApi == RenderSettings::API::VULKAN)
	{
		renderer = new VulkanRenderer();
	}
	else if (currentApi == RenderSettings::API::OPENGL)
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
		renderer->bindRenderSettings(renderSettings);
	}   

	assetImporter = std::make_unique<AssetImporter>(new AssimpModelImporter(), new StbImageImporter());
	assetBrowser = std::make_unique<AssetBrowser>();
	sceneGraphWindow = std::make_unique<SceneGraphWindow>();
	sceneGraph = std::make_unique<SceneGraph>();
	
	initInput(window);

	setSceneCamera(CameraType::ORBIT);

	auto light = std::make_shared<Light>(1, Light::Type::DIRECTIONAL);
	light->color = { 1.0f, 1.0f, 1.0f };
	light->position = { 1.8f, 0.9f, 0.0f };
	light->direction = { 0, -1.0f, 0 };
	light->constant = 1.0f;
	light->linear = 0.09f;
	light->quadratic = 0.032f;
	light->cutOff = 20.0f;
	light->outerCutOff = 27.0f;
	lightSources.push_back(light);

	auto light2 = std::make_shared<Light>(2, Light::Type::POINT);
	light2->color = { 1.0f, 1.0f, 1.0f };
	light2->position = { 0.0f, 0.9f, 1.8f };
	light2->direction = { 0.0f, 0, -1.0f };
	light2->constant = 1.0f;
	light2->linear = 0.09f;
	light2->quadratic = 0.032f;
	light2->cutOff = 20.0f;
	light2->outerCutOff = 27.0f;
	lightSources.push_back(light2);

	renderer->addLightSources(lightSources.data(), lightSources.size());

	return 0;
}

void Application::loadUserPrefs()
{
	renderSettings = std::make_unique<RenderSettings>();
	GetPrefs("Render_settings", *renderSettings);
}

void Application::saveUserPrefs()
{
	SavePrefs("Render_settings", *renderSettings);
}

void Application::processInput()
{
	IsUIFocused = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
	process(window);
}

void Application::update()
{
	camera->update();
}

void Application::render()
{
	renderer->draw();
}

void Application::cleanup()
{
	delete renderer;
	renderer = nullptr;
}

void Application::destroyWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::onCameraTypeChanged(CameraType newCameraType)
{
	setSceneCamera(cameraType);
}

void Application::onCameraSettingsChanged()
{
	this->camera->setFov(cameraFov);
}

void Application::onLightSettingsChanged()
{
	// nothing for now
}

void Application::setSceneCamera(CameraType cameraType)
{
	std::shared_ptr<BaseCamera> oldCamera = nullptr;
	if (this->camera != nullptr)
	{
		oldCamera = std::move(this->camera);
	}

	bool flipY = currentApi == RenderSettings::API::VULKAN;

	switch (this->cameraType)
	{
	case CameraType::ORBIT:
 		this->camera = std::make_shared<OrbitCamera>(cameraFov, Z_NEAR, Z_FAR, WINDOW_WIDTH, WINDOW_HEIGHT, flipY);
		break;
	case CameraType::FPV:
		this->camera = std::make_shared<FpvCamera>(cameraFov, Z_NEAR, Z_FAR, WINDOW_WIDTH, WINDOW_HEIGHT, flipY);
		break;
	}

	// Copy previous camera state and delete it
	if (oldCamera != nullptr)
	{
		EventBinder::Unbind(&BaseCamera::onMouseMove, oldCamera.get(), inp::onMouseMove);
		EventBinder::Unbind(&BaseCamera::onMouseScroll, oldCamera.get(), inp::onMouseScroll);
		EventBinder::Unbind(&BaseCamera::onKey, oldCamera.get(), inp::onKey);
	}

	EventBinder::Bind(&BaseCamera::onMouseMove, camera.get(), inp::onMouseMove);
	EventBinder::Bind(&BaseCamera::onMouseScroll, camera.get(), inp::onMouseScroll);
	EventBinder::Bind(&BaseCamera::onKey, camera.get(), inp::onKey);

	renderer->setCamera(camera);
}

void Application::cloneSceneInstance(uint32_t instanceId)
{
	const SceneGraphInstance& newInstance = sceneGraph->cloneInstance(instanceId);\
	renderer->addToRendererTextured(dynamic_cast<const ModelInstance&>(newInstance));
}

void Application::deleteSceneInstance(uint32_t instanceId)
{
	sceneGraph->deleteInstance(instanceId);
	renderer->removeFromRenderer(instanceId);
}

void Application::hideSceneInstance(uint32_t instanceId)
{
	std::cout << "hide" << std::endl;
}

void Application::onAssetBrowserAction(AssetBrowserOp action, std::string modelName)
{
	switch (action)
	{
	case AssetBrowserOp::ADD:
		addModelToRenderer(modelName);
		break;
	default:
		break;
	}
}

void Application::addModelToRenderer(std::string modelName)
{
	if (renderer != nullptr && sceneGraph != nullptr)
	{
		auto model = assetImporter->importModel(modelName);
		const SceneGraphInstance& newInstance = sceneGraph->addInstance(*model);
		renderer->addToRendererTextured(dynamic_cast<const ModelInstance&>(newInstance));
	}
}

void Application::onSceneGraphAction(SceneGraphOp action, uint32_t instanceId)
{
	switch(action)
	{
	case SceneGraphOp::CLONE :
		cloneSceneInstance(instanceId);
		break;
	case SceneGraphOp::DELETE:
		deleteSceneInstance(instanceId);
		break;
	case SceneGraphOp::HIDE:
		hideSceneInstance(instanceId);
		break;
	}
}

void Application::onInstanceTransformChanged(uint32_t id)
{
	const SceneGraphInstance& instance = sceneGraph->getInstance(id);
	renderer->updateModelTransform(instance.id, instance.getTransformMat());
}

void Application::imguiMenu()
{	
	// SETTINGS EDITOR WINDOW
	{
		ImGui::Begin("vRenderer Settings", nullptr, ImGuiWindowFlags_None);
		if (ImGui::BeginTabBar("Menus")) {
			if (ImGui::BeginTabItem("Inspector")) {
				assetBrowser->Draw(MODEL_ASSETS_FOLDER, c_supportedFormats, [this](AssetBrowserOp action, std::string model) {
					onAssetBrowserAction(action, model);
				});

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Renderer"))
			{
				imgui_helper::ShowRendererSettingsTab(*renderSettings.get());
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Camera"))
			{
				bool typeChanged, settingsChanged;
				imgui_helper::ShowCameraSettingsTab(cameraType, cameraFov, typeChanged, settingsChanged);
				if (typeChanged)
				{
					onCameraTypeChanged(cameraType);
				}
				if (settingsChanged)
				{
					onCameraSettingsChanged();
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Light"))
			{
				bool settingsChanged;
				imgui_helper::ShowLightSettingsTab(lightSources, settingsChanged);
				if (settingsChanged)
				{
					onLightSettingsChanged();
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();

		ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_None);
		sceneGraphWindow->Draw(*sceneGraph, [this](SceneGraphOp action, uint32_t instanceId) {
			onSceneGraphAction(action, instanceId);},
			[this](uint32_t instanceId) {
				onInstanceTransformChanged(instanceId);
			});

		ImGui::Separator();
		ImGui::End();
	}

	imgui_helper::DrawFPSOverlay(currentApi == RenderSettings::API::VULKAN ? "Vulkan" : "OpenGL");
}

int Application::run()
{
	loadUserPrefs();
	currentApi = renderSettings->api;

	initWindow(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);

	if (initApplication() == EXIT_FAILURE)
		return EXIT_FAILURE;

	float frameTime = 0;
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		if (currentApi == RenderSettings::API::OPENGL)
			glfwSwapBuffers(window);

		currentFrameTime = glfwGetTime() * 1000.0f;
		frameTime = currentFrameTime - previousFrameTime;

		float TARGET_FRAME_TIME = 1000 / renderSettings->targetFps;
		if (renderSettings->fpsLimit && frameTime < TARGET_FRAME_TIME) continue;

		context->deltaTime = frameTime / 1000.0f;
		previousFrameTime = currentFrameTime;

		processInput();
		update();
		render();
	}

	cleanup();
	saveUserPrefs();
	destroyWindow();
}