#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>

#include "ISceneInstanceTemplate.h"
#include "SceneGraphInstance.h"

namespace VRD::Scene
{
	class SceneGraph
	{
	public:

		SceneGraph() {}

		const SceneGraphInstance& addInstance(const ISceneInstanceTemplate& instanceTemplate)
		{
			uint32_t newInstanceId = global_nextId++;
			SceneGraphInstance* newInstance = createInstanceFromTemplate(newInstanceId, instanceTemplate);
			sceneInstances[newInstance->id] = newInstance;
			return *newInstance;;
		}

		const SceneGraphInstance& cloneInstance(uint32_t id)
		{
			const SceneGraphInstance& clonedInstance = *sceneInstances[id];
			uint32_t newInstanceId = global_nextId++;
			std::string newInstanceName = clonedInstance.printName + " Copy";
			SceneGraphInstance* newInstance = clonedInstance.clone(newInstanceId, newInstanceName);
			sceneInstances[newInstance->id] = newInstance;
			return *newInstance;
		}

		bool deleteInstance(const SceneGraphInstance& instance)
		{
			// TODO IMPLEMENT
		}

		const std::unordered_map<uint32_t, SceneGraphInstance*>& getInstances() const
		{
			return sceneInstances;
		}

		inline bool hasInstance(uint32_t id) const
		{
			return sceneInstances.find(id) != sceneInstances.end();
		}

		const SceneGraphInstance& getInstance(uint32_t id) const
		{
			return *sceneInstances.at(id);
		}

		SceneGraphInstance& getInstance(uint32_t id)
		{
			return *sceneInstances.at(id);
		}

	private:

		uint32_t global_nextId = 0;

		std::unordered_map<uint32_t, SceneGraphInstance*> sceneInstances = {};

		SceneGraphInstance* createInstanceFromTemplate(uint32_t id, const ISceneInstanceTemplate& instanceTemplate) const
		{
			std::string name = instanceTemplate.name + " #" + std::to_string(id);
			ModelInstance* modelInstance = new ModelInstance(id, name);
			modelInstance->createFromTemplate(dynamic_cast<const Model&>(instanceTemplate));
			return modelInstance;
		}
	};

}