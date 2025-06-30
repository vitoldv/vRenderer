#pragma once

#include "VkCubemap.h"
#include "VkSimpleMesh.h"
#include "VulkanUtils.h"
#include "VkSkyboxPipeline.h"
#include "BaseCamera.h"

using namespace VkUtils;

class VkSkybox
{
public:

	VkSkybox(const Cubemap& cubemap, VkContext context);
	~VkSkybox();

	void cmdDraw(VkCommandBuffer commandBuffer, const VkSkyboxPipeline& pipeline, const BaseCamera& camera);
	void cleanup();

private:

	VkSampler sampler;
	VkDescriptorSet samplerDescriptorSet;
	VkDescriptorPool samplerDescriptorPool;

	std::unique_ptr<VkCubemap> cubemap;
	std::unique_ptr<VkSimpleMesh> skyboxMesh;

	VkContext context;

	void createFromCubemap(const Cubemap& cubemap);
};