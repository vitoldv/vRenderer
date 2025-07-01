#pragma once

#include "IVkCoreResourceHolder.h"
#include "VkCubemap.h"
#include "VkSetLayoutFactory.h"
#include "VulkanUtils.h"

using namespace VkUtils;

class VkCubemapSamplerSet : IVkCoreResourceHolder
{
public:

	VkCubemapSamplerSet(const Cubemap& cubemap, VkContext context);
	~VkCubemapSamplerSet();

	void cmdBind(uint32_t setIndex, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	void cleanup();

private:

	VkSampler sampler;
	VkDescriptorSet samplerDescriptorSet;
	VkDescriptorPool samplerDescriptorPool;
	std::unique_ptr<VkCubemap> cubemap;
	VkContext context;

	void createFromCubemap(const Cubemap& cubemap);
};