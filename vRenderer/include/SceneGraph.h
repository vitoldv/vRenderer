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
			uint32_t instanceId = 0;

			auto it = templateIdInstanceCountMap.find(instanceTemplate.id);
			if (it != templateIdInstanceCountMap.end())
			{
				instanceId = templateIdInstanceCountMap[instanceTemplate.id]++;
			}
			else
			{
				templateIdInstanceCountMap[instanceTemplate.id]++;
			}


			SceneGraphInstance* newInstance = createInstance(instanceId, instanceTemplate);
			sceneInstances[newInstance->id] = newInstance;
			++instanceCount;

			return *newInstance;;
		}

		SceneGraphInstance* createInstance(uint32_t id, const ISceneInstanceTemplate& instanceTemplate) const
		{
			std::string name = instanceTemplate.name + " #" + std::to_string(id);
			ModelInstance* modelInstance = new ModelInstance(id, name);
			modelInstance->createFromTemplate(dynamic_cast<const Model&>(instanceTemplate));
			return modelInstance;
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
		uint32_t instanceCount = 0;
		std::unordered_map<uint32_t, SceneGraphInstance*> sceneInstances = {};

		// Key: template id. Value: instance count
		std::unordered_map<uint32_t, uint32_t> templateIdInstanceCountMap = {};
	};

}