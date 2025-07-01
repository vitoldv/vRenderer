#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <vector>
#include <list>

#include "RenderSettings.h"
#include "OpenGLRenderer.h"
#include "VulkanRenderer.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "AppContext.h"
#include "ThreadDispatcher.h"

#include "AssetImporter.h"
#include "AssimpModelImporter.h"
#include "StbImageImporter.h"

#include "SceneGraph.h"

#include "SceneGraphWindow.h"
#include "AssetBrowser.h"

#include "utils.h"
#include "editor_settings.h"

#include "OrbitCamera.h"
#include "FpvCamera.h"

using namespace VRD::OVERL;
using AssetBrowserOp = VRD::OVERL::AssetBrowser::Op;
using SceneGraphOp = VRD::OVERL::SceneGraphWindow::Op;

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
	std::unique_ptr<ThreadDispatcher> threadDispatcher;

	// Currently utilized graphics api. The one in renderSettings is one selected by user and will be applied after app restart.
	RenderSettings::API currentApi;

	// Frame time control 
	int previousFrameTime = 0;
	int currentFrameTime = 0;

	CameraType cameraType;
	int cameraFov = 70;

	std::shared_ptr<BaseCamera> camera;
	std::vector<std::shared_ptr<Light>> lightSources;

	std::unique_ptr<AssetImporter> assetImporter;
	std::unique_ptr<SceneGraph> sceneGraph;

	std::unique_ptr<AssetBrowser> assetBrowser;
	std::unique_ptr<SceneGraphWindow> sceneGraphWindow;

	std::shared_ptr<Cubemap> skyboxCubemap;

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

	void createModelInstance();
	void cloneSceneInstance(uint32_t instanceId);
	void deleteSceneInstance(uint32_t instanceId);
	void hideSceneInstance(uint32_t instanceId);

	void onCameraTypeChanged(CameraType cameraType);
	void onCameraSettingsChanged();
	void onLightSettingsChanged();
	void onAssetBrowserAction(AssetBrowserOp action, std::string modelName);
	void onSceneGraphAction(SceneGraphOp action, uint32_t instanceId);
	void onInstanceTransformChanged(uint32_t id);
	void addModelToRenderer(std::string modelName);


	void setSceneCamera(CameraType cameraType);
};