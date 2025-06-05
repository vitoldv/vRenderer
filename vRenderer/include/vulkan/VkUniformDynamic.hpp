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

	VkUniformDynamic(uint32_t descriptorSetIndex, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkContext context);
	~VkUniformDynamic();

	size_t getBufferSize() const;
	void update(uint32_t imageIndex, const T* data, uint32_t drawCount);
	void updateAll(const T* data, uint32_t drawCount);
	void cmdBind(uint32_t imageIndex, uint32_t drawIndex, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	const VkDescriptorSetLayout getDescriptorLayout() const;

	void cleanup();

private:

	const uint32_t imageCount;
	const uint32_t descriptorSetIndex;

	T* transferSpace;
	size_t alignment;

	std::vector<VkBuffer> buffer;
	std::vector<VkDeviceMemory> memory;
	std::vector<VkDescriptorSet> descriptorSet;

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkContext context;

	void allocateDynamicBufferTransferSpace();
	void create();
};

#include "VkUniformDynamic.ipp"