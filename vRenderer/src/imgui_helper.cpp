#include "imgui/imgui_helper.h"

namespace imgui_helper
{
	const char* apiLabels[] = { "Vulkan", "OpenGL" };
	const char* cameraTypeLabels[] = { "Orbit", "FPV" };
	const char* lightTypeLabels[] = { "Directional", "Point", "Spot" };

	template<typename Enum>
	void EnumButtonGroup(const char* labels[], int count, Enum& value, bool& changed)
	{
		ImGui::BeginGroup();
		for (int i = 0; i < count; i++) {
			bool isActive = (static_cast<int>(value) == i);
			if (isActive) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
			}

			if (ImGui::Button(labels[i], ImVec2(80, 0)))
			{
				changed = true;
				value = static_cast<Enum>(i);
			}

			if (isActive) ImGui::PopStyleColor(2);
			if (i < count - 1) ImGui::SameLine();
		}
		ImGui::EndGroup();
	}

	void ShowRendererSettingsTab(RenderSettings& renderSettings)
	{
		ImGui::Text("API	");
		bool changed;
		EnumButtonGroup<RenderSettings::API>(apiLabels, 2, renderSettings.api, changed);

		ImGui::Text("General");
		ImGui::Checkbox("FPS limit", &renderSettings.fpsLimit);
		ImGui::SliderInt("FPS target", &renderSettings.targetFps, 1, 165);
		ImGui::Checkbox("Object outline", &renderSettings.enableOutline);
		ImGui::DragFloat("Gamma Correction factor", &renderSettings.gammaCorrectionFactor, 0.1f, 5.0f);
	}

	void ShowCameraSettingsTab(CameraType& cameraType, int& fov, bool& typeChanged, bool& settingsChanged)
	{
		typeChanged = false;
		settingsChanged = false;
		EnumButtonGroup<CameraType>(cameraTypeLabels, 2, cameraType, typeChanged);
		settingsChanged = ImGui::SliderInt("FOV", &fov, 1, 150);
	}

	void ShowLightSettingsTab(const std::vector<std::shared_ptr<Light>>& lights, std::function<void(LightTabAction, uint32_t)> callback)
	{
		if (ImGui::Button("Add"))
		{
			callback(LightTabAction::Add, -1);
		}

		for (int i = 0; i < lights.size(); i++)
		{
			Light* light = lights[i].get();
			bool settingsChanged = false;
			std::string header = "Source #" + std::to_string(i) + " (" + lightTypeLabels[light->type] + ")";
			if (ImGui::CollapsingHeader(header.c_str()))
			{
				ImGui::PushID(i);
				bool typeChanged = false;
				EnumButtonGroup(lightTypeLabels, 3, light->type, typeChanged);
				settingsChanged |= typeChanged;
				settingsChanged |= ImGui::DragFloat3("Position", static_cast<float*>(&light->position[0]), 0.1f, -100.0f, 100.0f);
				settingsChanged |= ImGui::DragFloat3("Direction", static_cast<float*>(&light->direction[0]), 0.1f, -100.0f, 100.0f);
				settingsChanged |= ImGui::DragFloat("Constant", &light->constant, 0.05f, 0, 1.0f);
				settingsChanged |= ImGui::DragFloat("Linear", &light->linear, 0.05f, 0, 1.0f);
				settingsChanged |= ImGui::DragFloat("Quadratic", &light->quadratic, 0.05f, 0, 1.0f);
				settingsChanged |= ImGui::DragFloat("CutOff", &light->cutOff, 0.05f, 0, 120.0f);
				settingsChanged |= ImGui::DragFloat("OuterCutOff", &light->outerCutOff, 0.05f, 0, 120.0f);
				ImGui::NewLine();
				settingsChanged |= ImGui::ColorPicker3("Color", static_cast<float*>(&light->color[0]));
				if (ImGui::Button("Remove"))
				{
					callback(LightTabAction::Remove, light->id);
				}
				ImGui::PopID();
			}
		}
	}

	/// <summary>
	/// Editor for a glm::vec3. Allows simultaneous edit of all components at once via sync flag.
	/// </summary>
	bool ShowAdvancedVec3Editor(const char* label, glm::vec3& vec, bool& sync,
		float step, float min, float max)
	{
		bool changed = true;
		ImGui::PushID(label);
		// Label and sync checkbox
		ImGui::Text("%s", label);
		ImGui::SameLine();
		ImGui::Checkbox("Sync", &sync);

		// Helper lambda for synchronized editing
		auto SyncValues = [&vec](int changedComponent) {
			float newVal = vec[changedComponent];
			vec.x = vec.y = vec.z = newVal;
			};

		const char* components[] = { "X", "Y", "Z" };
		float lineHeight = ImGui::GetFrameHeight();
		float itemWidth = ImGui::GetContentRegionAvail().x * 0.15f;

		for (int i = 0; i < 3; i++) {
			ImGui::PushID(i);
			// Component label
			ImGui::Text("%s:", components[i]);
			ImGui::SameLine();
			// Input field
			ImGui::SetNextItemWidth(itemWidth);
			if (ImGui::DragFloat("##value", &vec[i], step, min, max, "%.3f")) {
				changed = true;
				if (sync) SyncValues(i);
			}
			ImGui::SameLine();
			// Slider
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 10.0f);
			if (ImGui::SliderFloat("##slider", &vec[i], min, max, "%.3f")) 
			{
				changed = true;
				if (sync) SyncValues(i);
			}
			ImGui::PopID();
		}
		ImGui::PopID();

		return changed;
	}

	/// <summary>
	/// Editor for tranform values of a mesh/model
	/// </summary>
	bool ShowTransformEditor(SceneGraphInstance& sceneInstance)
	{
		bool changed = false;
		static bool syncPosition = false;
		static bool syncRotation = false;
		static bool syncScale = false;

		// Position (with wider range and default speed)
		changed |= ShowAdvancedVec3Editor("Position", sceneInstance.position, syncPosition, 0.01f, -100.0f, 100.0f);
		ImGui::Spacing();
		// Rotation (constrained to 0-360 range, faster speed)
		changed |= ShowAdvancedVec3Editor("Rotation", sceneInstance.rotation, syncRotation, 0.01f, -360.0f, 360.0f);
		ImGui::Spacing();
		// Scale (positive values only, slower speed)
		changed |= ShowAdvancedVec3Editor("Scale", sceneInstance.scale, syncScale, 0.01f, 0, 20.0f);

		return changed;
	}

	void DrawFPSOverlay(const char* info)
	{
		// Set the position to the top-left corner (with a small offset for padding)
		const float DISTANCE = 10.0f;
		ImVec2 window_pos = ImVec2(DISTANCE, DISTANCE);
		ImVec2 window_pos_pivot = ImVec2(0.0f, 0.0f);

		// Use a transparent, borderless, non-interactive window
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration |      // No title bar, resize, etc.
			ImGuiWindowFlags_AlwaysAutoResize |  // Auto-resize to fit content
			ImGuiWindowFlags_NoSavedSettings |   // Don't remember position/size
			ImGuiWindowFlags_NoFocusOnAppearing |// Don't steal focus
			ImGuiWindowFlags_NoNav |             // No keyboard/gamepad nav
			ImGuiWindowFlags_NoMove;             // Lock position

		// Begin the overlay
		if (ImGui::Begin("FPS Overlay", nullptr, flags)) {
			// Just display the FPS number
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
			ImGui::Text(info);
		}
		ImGui::End();
	}
}