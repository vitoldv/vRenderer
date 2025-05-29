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

private:
	//std::vector<VkBuffer> lightUniformBuffers;
	//std::vector<VkDeviceMemory> lightUniformMemory;
	//VkDescriptorSetLayout vkLightDescriptorSetLayout;
	//std::vector<VkDescriptorSet> vkLightDescriptorSets;

	VkBuffer buffer;
	VkDeviceMemory memory;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;

	VkShaderStageFlagBits shaderStage;
	VkContext context;

	void create();
};

#include "VkUniform.ipp"