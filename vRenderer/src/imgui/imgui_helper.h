#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <string>
#include <vector>
#include <filesystem>

#include <glm/glm.hpp>

namespace imgui_helper
{
	static std::vector<std::string> modelNames;
	static int selectedModelIndex = -1;

	void ScanForModels(const std::string& directory, const std::vector<const char*>& extensions, std::vector<std::string>& outModelNames)
	{
		namespace fs = std::filesystem;

		outModelNames.clear();
		if (directory.empty() || !fs::exists(directory) || !fs::is_directory(directory))
			return;

		for (const auto& entry : fs::directory_iterator(directory))
		{
			if (entry.is_directory())
			{
				bool found = false;
				for (const auto& file : fs::directory_iterator(entry))
				{
					if (!file.is_regular_file()) continue;

					std::string ext = file.path().extension().string();
					for (const auto& supported : extensions)
					{
						if (ext == supported)
						{
							outModelNames.push_back(entry.path().filename().string());
							found = true;
							break;
						}
					}
					if (found) break;
				}
			}
		}
	}

	void DrawAssetBrowser(const std::string& assetsFolderPath, const std::vector<const char *>& supportedExtensions, std::string& selectedModelName, bool& newSelection)
	{
		ImGui::Begin("Asset Browser");

		// UI: trigger to rescan models
		if (ImGui::Button("Scan Folder"))
		{
			ScanForModels(assetsFolderPath, supportedExtensions, modelNames);
			selectedModelIndex = -1;
			selectedModelName = "";
		}

		ImGui::Separator();
		ImGui::Text("Supported Extensions:");
		for (auto& ext : supportedExtensions)
		{
			ImGui::SameLine();
			ImGui::TextUnformatted(ext);
		}

		ImGui::Separator();
		ImGui::Text("Model Folders:");
		ImGui::BeginChild("ModelList", ImVec2(0, 200), true);

		for (int i = 0; i < modelNames.size(); ++i)
		{
			bool isSelected = (selectedModelIndex == i);
			if (ImGui::Selectable(modelNames[i].c_str(), isSelected))
			{
				newSelection = true;
				selectedModelIndex = i;
				selectedModelName = modelNames[i];
			}
		}

		ImGui::EndChild();

		ImGui::Separator();
		ImGui::Text("Selected Model: %s", selectedModelName.empty() ? "<none>" : selectedModelName.c_str());

		ImGui::End();
	}


	/// <summary>
	/// Editor for a glm::vec3. Allows simultaneous edit of all components at once via sync flag.
	/// </summary>
	void ShowAdvancedVec3Editor(const char* label, glm::vec3& vec, bool& sync,
		float speed = 0.1f, float min = -100.0f, float max = 100.0f)
	{
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
			if (ImGui::DragFloat("##value", &vec[i], speed, min, max, "%.3f")) {
				if (sync) SyncValues(i);
			}
			ImGui::SameLine();
			// Slider
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 10.0f);
			if (ImGui::SliderFloat("##slider", &vec[i], min, max, "%.3f")) {
				if (sync) SyncValues(i);
			}
			ImGui::PopID();
		}
		ImGui::PopID();
	}

	/// <summary>
	/// Editor for tranform values of a mesh/model
	/// </summary>
	void ShowTransformEditor(glm::vec3& position, glm::vec3& rotation, glm::vec3& scale)
	{
		static bool syncPosition = false;
		static bool syncRotation = false;
		static bool syncScale = false;

		ImGui::Begin("Transform Editor");
		// Position (with wider range and default speed)
		ShowAdvancedVec3Editor("Position", position, syncPosition, 0.1f, -100.0f, 100.0f);
		ImGui::Spacing();
		// Rotation (constrained to 0-360 range, faster speed)
		ShowAdvancedVec3Editor("Rotation", rotation, syncRotation, 1.0f, 0.0f, 360.0f);
		ImGui::Spacing();
		// Scale (positive values only, slower speed)
		ShowAdvancedVec3Editor("Scale", scale, syncScale, 0.05f, 0.0f, 100.0f);
		ImGui::End();
	}

	/// <summary>
	/// Editor for tranform values of a mesh/model
	/// </summary>
	void ShowCameraEditor(glm::vec3& position, float& fov)
	{
		static bool syncPosition = false;

		ImGui::Begin("Camera Editor");
		ShowAdvancedVec3Editor("Position", position, syncPosition, 0.1f, -100.0f, 100.0f);
		ImGui::Spacing();
		ImGui::DragFloat("FOV", &fov, 1.0f, 200.0f);
		ImGui::Spacing();

		ImGui::End();
	}
}