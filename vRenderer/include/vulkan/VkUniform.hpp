#pragma once

#include <vector>
#include <memory>
#include "VulkanUtils.h"

using namespace VkUtils;

template<typename T>
class VkUniform
{
public:

	const uint32_t descriptorSetIndex;

	VkUniform(uint32_t descriptorSetIndex, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkContext context);
	~VkUniform();

	void update(const T& data);
	void cmdBind(uint32_t setNumber, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	const VkDescriptorSetLayout getDescriptorLayout() const;

	void cleanup();

protected:

	VkBuffer buffer;
	VkDeviceMemory memory;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;
	VkDescriptorPool descriptorPool;

	VkContext context;

	void create();

	size_t getBufferSize();
};

#include "VkUniform.ipp"