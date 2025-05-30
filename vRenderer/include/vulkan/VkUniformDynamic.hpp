#pragma once

#include <vector>
#include <memory>
#include "VulkanUtils.h"

/// <summary>
/// Wrapper for Vulkan's dynamic uniforms.
/// P.S. Though a lot of logic duplicated one of VkUniform, I kept it separate to keep
/// these two uniform types distinct. Unification in some form might be considered in future
/// if changes in one class would be followed by the same changes in another.
/// </summary>
/// <typeparam name="T"></typeparam>
template<typename T>
class VkUniformDynamic
{
public:

	VkUniformDynamic(VkShaderStageFlagBits shaderStageFlags, VkContext context);
	~VkUniformDynamic();

	size_t getBufferSize() const;
	void update(const T* data, uint32_t drawCount);
	void cmdBind(uint32_t drawCount, uint32_t setNumber, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	const VkDescriptorSetLayout getDescriptorLayout() const;

	void cleanup();

private:

	T* transferSpace;
	size_t alignment;

	VkBuffer buffer;
	VkDeviceMemory memory;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;

	VkContext context;
	VkShaderStageFlagBits shaderStage;

	void allocateDynamicBufferTransferSpace();
	void create();
};

#include "VkUniformDynamic.ipp"