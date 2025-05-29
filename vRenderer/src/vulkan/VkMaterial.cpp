#include "VkMaterial.h"

VkMaterial::VkMaterial(const Material& material, VkContext context, VkSamplerDescriptorSetCreateInfo createInfo)
{
	diffuse = nullptr;
	specular = nullptr;
	this->context = context;

	createFromGenericMaterial(material, createInfo);
}

VkMaterial::~VkMaterial()
{
	cleanup();
}

const std::vector<VkDescriptorSet>& VkMaterial::getSamplerDescriptorSets() const
{
	return samplerDescriptorSets;
}

/// <summary>
/// Creates VkTexture along with sampler descriptor set for each specific texture type if one present in template material.
/// If texture type is not present - a corresponding null descriptor is created.
/// </summary>
/// <param name="material"></param>
/// <param name="createInfo"></param>
void VkMaterial::createFromGenericMaterial(const Material& material, VkSamplerDescriptorSetCreateInfo createInfo)
{
	// Create sampler pool
	samplerDescriptorPool = createDescriptorPool(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		textureCount,
		VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		context);
	createInfo.descriptorPool = samplerDescriptorPool;

	// Lambda for texture creation
	VkContext& ctx = this->context;
	auto createTexture = [&](std::string textureStr, std::unique_ptr<VkTexture>& vkTexture) {
		if (!textureStr.empty())
		{
			vkTexture = std::make_unique<VkTexture>(textureStr, ctx);
		}
	};

	// Lambda for per-texture descriptor set creation
	auto& descriptors = samplerDescriptorSets;
	auto createDescriptor = [&](std::unique_ptr<VkTexture>& vkTexture) {
		VkDescriptorSet descriptorSet = vkTexture != nullptr
			? vkTexture->createSamplerDescriptor(createInfo)
			: createNullSamplerDescriptor(createInfo);
		descriptors.push_back(descriptorSet);
	};

	createTexture(material.diffuseTexture, diffuse);
	createTexture(material.specularTexture, specular);

	createDescriptor(diffuse);
	createDescriptor(specular);
}

/// <summary>
/// Creates generic null sampler descriptor. Create on for empty texture and bind to pipeline.
/// P.S. Requires nullDescriptors feature enabled in VkPhysicalDeviceRobustness2FeaturesEXT.
/// </summary>
/// <param name="createInfo"></param>
/// <returns></returns>
VkDescriptorSet VkMaterial::createNullSamplerDescriptor(VkSamplerDescriptorSetCreateInfo createInfo)
{
	VkDescriptorSet dummySet;

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = createInfo.descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &createInfo.samplerDescriptorSetLayout;

	VkResult result = vkAllocateDescriptorSets(context.logicalDevice, &allocInfo, &dummySet);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate texture sampler descriptor sets.");
	}

	uint32_t dummyBufferSize = 16;

	VkBuffer dummyBuffer;
	VkDeviceMemory dummyBufferMemory;
	VkUtils::createBuffer(context.physicalDevice, context.logicalDevice, dummyBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &dummyBuffer, &dummyBufferMemory);

	// dummy image info (null descriptor feature should be enabled)
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.imageView = VK_NULL_HANDLE;
	imageInfo.sampler = createInfo.sampler;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = dummySet;
	descriptorWrite.dstBinding = 0; // Matches layout binding
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(context.logicalDevice, 1, &descriptorWrite, 0, nullptr);

	dummyBuffers.push_back(dummyBuffer);
	dummyBuffersMemory.push_back(dummyBufferMemory);

	return dummySet;
}

void VkMaterial::cleanup()
{
	vkDestroyDescriptorPool(context.logicalDevice, samplerDescriptorPool, nullptr);
	for (int i = 0; i < dummyBuffers.size(); i++)
	{
		vkDestroyBuffer(context.logicalDevice, dummyBuffers[i], nullptr);
		vkFreeMemory(context.logicalDevice, dummyBuffersMemory[i], nullptr);
	}
}
