#include "VkModelBatch.h"

 VkModelBatch::VkModelBatch(uint32_t initialCapacity, VkModel* model, VkContext context) : modelId(model->id)
{
	this->context = context;
	this->model.reset(model);
	this->capacity = initialCapacity;

	instanceCount = 0;

	// Size of buffer needed for all vertex buffers
	VkDeviceSize bufferSize = sizeof(InstanceData) * initialCapacity;

	createBuffer(context.physicalDevice, context.logicalDevice, bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&instanceDataBuffer, &instanceBufferMemory);

	void* data;
	vkMapMemory(context.logicalDevice, instanceBufferMemory, 0, bufferSize, 0, &data);		// "map" the vertex buffer memory to some point
	pInstanceData = static_cast<InstanceData*>(data);
}

 void VkModelBatch::cmdDraw(uint32_t imageIndex, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, bool bindMaterials)
{
	// Bind per instance data
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 1, 1, &instanceDataBuffer, offsets);

	uint32_t meshCount = model->getMeshCount();
	for (int i = 0; i < meshCount; ++i)
	{
		model->cmdBindSubMesh(imageIndex, i, commandBuffer, pipelineLayout, bindMaterials);
		// execute pipeline
		vkCmdDrawIndexed(commandBuffer, model->getMeshIndexCount(i), instanceCount, 0, -VERTEX_INDEX_OFFSET, 0);
	}
}

/// <summary>
/// Add instance with "id" to batch. 
/// If there is room for instance an pointer to its data memory is returned 
/// </summary>
/// <param name="id"></param>
/// <returns></returns>
 VkModelBatch::InstanceData* VkModelBatch::addInstance(instanceId_t id)
{
	if (instanceCount + 1 <= capacity)
	{
		InstanceData* pSharedMem = pInstanceData + instanceCount;
		instanceIndexMap[id] = pSharedMem;
		return pSharedMem;
	}

	return nullptr;
}


/// <summary>
/// Add instance with "id" to batch. 
/// If there is room for instance, the provided instance data is copied to correct memory location
/// </summary>
/// <param name="id"></param>
/// <returns></returns>
 bool VkModelBatch::addInstance(instanceId_t id, InstanceData* pData)
{
	if (instanceCount + 1 <= capacity)
	{
		InstanceData* pSharedMem = pInstanceData + instanceCount;
		memcpy(static_cast<void*>(pSharedMem), static_cast<void*>(pData), sizeof(InstanceData));
		instanceIndexMap[id] = pSharedMem;
		++instanceCount;
		return true;
	}
	return false;
}

 bool VkModelBatch::updateInstanceData(instanceId_t id, InstanceData* pData)
{
	if (instanceIndexMap.find(id) != instanceIndexMap.end())
	{
		*instanceIndexMap[id] = *pData;
		return true;
	}
	return false;
}

 void VkModelBatch::cleanup()
{
	vkUnmapMemory(context.logicalDevice, instanceBufferMemory);
	vkDestroyBuffer(context.logicalDevice, instanceDataBuffer, nullptr);
	vkFreeMemory(context.logicalDevice, instanceBufferMemory, nullptr);
}
