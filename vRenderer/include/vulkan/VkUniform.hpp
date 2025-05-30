#pragma once

#include <vector>
#include <memory>
#include "VulkanUtils.h"

using namespace VkUtils;

template<typename T>
class VkUniform
{
public:

	VkUniform(VkShaderStageFlagBits shaderStageFlags, VkContext context);
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

	VkContext context;
	VkShaderStageFlagBits shaderStage;

	void create();

	size_t getBufferSize();
};

#include "VkUniform.ipp"