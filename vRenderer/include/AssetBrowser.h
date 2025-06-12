#pragma once

#include <vector>
#include <string>

#include "imgui.h"


namespace VRD::OVERL
{

	class AssetBrowser
	{

	public:

		enum class Op
		{
			ADD
		};

		AssetBrowser() {}

		template<typename Callback>
		void Draw(const std::string& assetsFolderPath, const std::vector<const char*>& supportedExtensions, Callback callback)
		{
			ScanForModels(assetsFolderPath, supportedExtensions, modelNames);

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
					selectedModelIndex = i;
					selectedModelName = modelNames[i];
				}
			}

			ImGui::EndChild();

			ImGui::Separator();
			ImGui::Text("Selected Model: %s", selectedModelName.empty() ? "<none>" : selectedModelName.c_str());
			ImGui::SameLine();
			if (ImGui::Button("Add to scene"))
			{
				if (!selectedModelName.empty())
				{
					callback(Op::ADD, selectedModelName);
				}
			}
			modelNames.data();
		}

	private:

		std::vector<std::string> modelNames = {};
		std::string selectedModelName = {};
		uint32_t selectedModelIndex = -1;

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
	};
}