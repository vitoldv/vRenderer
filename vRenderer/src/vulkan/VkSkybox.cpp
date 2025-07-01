#include "VkSkybox.h"

VkSkybox::VkSkybox(const Cubemap& cubemap, VkContext context)
{
	this->context = context;
	skyboxMesh = std::make_unique<VkSimpleMesh>(SKYBOX_RESERVED_ID, VK_PRIMITIVE_MESH::CUBE, context);
	cubemapSampler = std::make_unique<VkCubemapSamplerSet>(cubemap, context);
}

VkSkybox::~VkSkybox()
{
	//cleanup();
}

void VkSkybox::cleanup()
{
	skyboxMesh->cleanup();
	cubemapSampler->cleanup();
}

void VkSkybox::cmdDraw(VkCommandBuffer commandBuffer, const VkSkyboxPipeline& pipeline)
{
	cubemapSampler->cmdBind(1, commandBuffer, pipeline.getLayout());
	skyboxMesh->cmdDraw(commandBuffer);
}

