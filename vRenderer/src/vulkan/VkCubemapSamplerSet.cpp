#include "VkCubemapSamplerSet.h"

VkCubemapSamplerSet::VkCubemapSamplerSet(const Cubemap& cubemap, VkContext context)
{
	this->context = context;
	// Create skybox
	createFromCubemap(cubemap);
}

VkCubemapSamplerSet::~VkCubemapSamplerSet()
{
	//cleanup();
}

void VkCubemapSamplerSet::cmdBind(uint32_t setIndex, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, setIndex, 1,
		&samplerDescriptorSet, 0, nullptr);
}

void VkCubemapSamplerSet::cleanup()
{
	cubemap->cleanup();
	vkDestroySampler(context.logicalDevice, sampler, nullptr);
	vkDestroyDescriptorPool(context.logicalDevice, samplerDescriptorPool, nullptr);
}

void VkCubemapSamplerSet::createFromCubemap(const Cubemap& cubemap)
{
	this->cubemap = std::make_unique<VkCubemap>(cubemap, context);

	// Create cubemap sampler pool
	samplerDescriptorPool = createDescriptorPool(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		1,
		VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		context);

	VkSamplerCreateInfo skyboxSamplerInfo = {};
	skyboxSamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	skyboxSamplerInfo.magFilter = VK_FILTER_LINEAR;
	skyboxSamplerInfo.minFilter = VK_FILTER_LINEAR;
	skyboxSamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	skyboxSamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	skyboxSamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	skyboxSamplerInfo.unnormalizedCoordinates = VK_FALSE;
	skyboxSamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	skyboxSamplerInfo.mipLodBias = 0.0f;
	skyboxSamplerInfo.maxLod = 0.0f;
	skyboxSamplerInfo.minLod = 0.0f;
	skyboxSamplerInfo.anisotropyEnable = VK_FALSE;

	VkResult result = vkCreateSampler(context.logicalDevice, &skyboxSamplerInfo, nullptr, &sampler);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create skybox sampler.");
	}

	// DESCRIPTOR
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		VkDescriptorSetLayout samplerSetLayout = VkSetLayoutFactory::instance().getSetLayout(DESC_SET_LAYOUT::CUBEMAP_SAMPLER);
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = samplerDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &samplerSetLayout;
		VkResult result = vkAllocateDescriptorSets(context.logicalDevice, &allocInfo, &samplerDescriptorSet);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate cubemap sampler descriptor sets.");
		}

		// texture image info
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = this->cubemap->getImageView();
		imageInfo.sampler = sampler;

		VkWriteDescriptorSet setWrite = {};
		setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		setWrite.descriptorCount = 1;
		setWrite.dstSet = samplerDescriptorSet;
		setWrite.dstArrayElement = 0;
		setWrite.dstBinding = 0;
		setWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(context.logicalDevice, 1, &setWrite, 0, nullptr);
	}
}
