#pragma once

#include "VulkanUtils.h"
#include "VkSimpleMesh.h"
#include "VkSkyboxPipeline.h"
#include "VkCubemapSamplerSet.h"

using namespace VkUtils;

class VkSkybox : IVkCoreResourceHolder
{
public:

	VkSkybox(const Cubemap& cubemap, VkContext context);
	~VkSkybox();

	void cmdDraw(VkCommandBuffer commandBuffer, const VkSkyboxPipeline& pipeline);
	void cleanup();

private:

	std::unique_ptr<VkCubemapSamplerSet> cubemapSampler;
	std::unique_ptr<VkSimpleMesh> skyboxMesh;
	VkContext context;
};