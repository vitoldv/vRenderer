#pragma once

#include <iostream>
#include <string>
#include <memory>

#include "RenderSettings.h"
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

class Application
{
public:
	Application() = default;

	int run();

private:

	GLFWwindow* window;
	IRenderer* renderer;
	std::shared_ptr<RenderSettings> renderSettings;
	AppContext* context;

	// Currently utilized graphics api. The one in renderSettings is one selected by user and will be applied after app restart.
	RenderSettings::API currentApi;

	// Frame time control 
	int previousFrameTime = 0;
	int currentFrameTime = 0;

	CameraType cameraType;
	int cameraFov = 70;

	// Scene
	std::string selectedModelName;
	bool newSelection;

	std::shared_ptr<BaseCamera> camera;
	std::vector<std::shared_ptr<Light>> lightSources;

	Model* modelToRender;
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);


	void initWindow(std::string title, const int width, const int height);
	int initApplication();
	void loadUserPrefs();
	void saveUserPrefs();
	void processInput();
	void update();
	void render();
	void cleanup();
	void destroyWindow();
	void imguiMenu();

	void onCameraTypeChanged(CameraType cameraType);
	void onCameraSettingsChanged();
	void onLightSettingsChanged();

	void setSceneCamera(CameraType cameraType);
};