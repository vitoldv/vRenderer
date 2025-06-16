#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>

#include "imgui.h"
#include "imgui/imgui_helper.h"
#include "SceneGraph.h"

namespace VRD::OVERL
{
	class SceneGraphWindow
	{
	public:
		enum class Op
		{
			SELECT,
			CLONE,
			DELETE,
			HIDE
		};

		SceneGraphWindow()
		{
			selectedId = std::numeric_limits<uint32_t>::max();
		}

		void Draw(SceneGraph& sceneGraph, std::function<void(Op action, uint32_t instanceId)> onAction, std::function<void(uint32_t instanceId)> onTransformChanged)
		{
			const auto& sceneGraphInstances = sceneGraph.getInstances();

			ImGui::Separator();

			ImGui::SameLine();
			if (ImGui::Button("Clone"))
			{
				if (sceneGraph.hasInstance(selectedId))
				{
					onAction(Op::CLONE, selectedId);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete"))
			{
				if (sceneGraph.hasInstance(selectedId))
				{
					onAction(Op::DELETE, selectedId);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Hide"))
			{
				if (sceneGraph.hasInstance(selectedId))
				{
					onAction(Op::HIDE, selectedId);
				}
			}
			

			ImGui::BeginChild("Scene graph", ImVec2(0, 200), true);
			for (const auto& pair : sceneGraphInstances)
			{
				const auto& instance = pair.second;
				if (ImGui::Selectable(instance->printName.c_str(), selectedId == instance->id))
				{
					selectedId = instance->id;
					onAction(Op::SELECT, selectedId);
				}
			}
			ImGui::EndChild();

			if (sceneGraph.hasInstance(selectedId))
			{
				SceneGraphInstance& instance = sceneGraph.getInstance(selectedId);
				if (imgui_helper::ShowTransformEditor(instance))
				{
					onTransformChanged(selectedId);
				}
			}
		}

	private:
		uint32_t selectedId;
	};
}