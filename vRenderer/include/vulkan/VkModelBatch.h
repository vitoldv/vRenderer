#pragma once

#include <unordered_map>

#include "IVkCoreResourceHolder.h"
#include "VulkanUtils.h"
#include "VkModel.h"

using namespace VkUtils;

class VkModelBatch : IVkCoreResourceHolder
{
public:

	struct InstanceData
	{
		glm::mat4 transform;
		glm::mat4 normalMatrix;

		InstanceData(glm::mat4 transform)
		{
			this->transform = transform;
			this->normalMatrix = glm::transpose(glm::inverse(transform));
		}
	};

	const modelId_t modelId;	

	VkModelBatch(uint32_t initialCapacity, VkModel* model, VkContext context);

	void cmdDraw(uint32_t imageIndex, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, bool bindMaterials);
	InstanceData* addInstance(instanceId_t id);
	bool addInstance(instanceId_t id, InstanceData* pData);
	bool updateInstanceData(instanceId_t id, InstanceData* pData);
	void cleanup();
	
private:

	uint32_t instanceCount;
	uint32_t capacity;
	std::unique_ptr<VkModel> model;
	InstanceData* pInstanceData = nullptr;
	// intance id - instance data ptr
	std::unordered_map<instanceId_t, InstanceData*> instanceIndexMap;

	VkBuffer instanceDataBuffer;
	VkDeviceMemory instanceBufferMemory;
	VkContext context;
};
