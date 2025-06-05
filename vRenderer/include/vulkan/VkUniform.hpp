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

	VkUniform(uint32_t imageCount, uint32_t descriptorSetIndex, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkContext context);
	~VkUniform();

	void update(uint32_t imageIndex, const T& data);
	void updateAll(const T& data);
	void cmdBind(uint32_t imageIndex, uint32_t setNumber, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const;

	void cleanup();

protected:

	const uint32_t imageCount;

	std::vector<VkBuffer> buffer;
	std::vector<VkDeviceMemory> memory;
	std::vector<VkDescriptorSet> descriptorSet;

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;

	VkContext context;

	void create();

	size_t getBufferSize();
};

#include "VkUniform.ipp"