#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <string>
#include <vector>
#include <filesystem>
#include <functional>
#include <numeric>

#include <glm/glm.hpp>

#include "ModelInstance.h"
#include "Lighting.h"
#include "RenderSettings.h"
#include "BaseCamera.h"

using namespace VRD::Scene;

namespace imgui_helper
{
	extern const char* apiLabels[];
	extern const char* cameraTypeLabels[];
	extern const char* lightTypeLabels[];

	template<typename Enum>
	void EnumButtonGroup(const char* labels[], int count, Enum& value, bool& changed);

	void ShowRendererSettingsTab(RenderSettings& renderSettings);
	void ShowCameraSettingsTab(CameraType& cameraType, int& fov, bool& typeChanged, bool& settingsChanged);
	void ShowLightSettingsTab(const std::vector<std::shared_ptr<Light>>& lights, bool& settingsChanged);

	/// <summary>
	/// Editor for a glm::vec3. Allows simultaneous edit of all components at once via sync flag.
	/// </summary>
	bool ShowAdvancedVec3Editor(const char* label, glm::vec3& vec, bool& sync,
		float speed = 0.1f, float min = -100.0f, float max = 100.0f);

	/// <summary>
	/// Transform editor for a SceneGraphInstance
	/// </summary>
	bool ShowTransformEditor(SceneGraphInstance& sceneInstance);

	// Draws current framerate based on IMGUI data
	void DrawFPSOverlay(const char* info = "");
}