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

/// <summary>
/// Creates VkTexture along with sampler descriptor sets for each specific texture type if one present in template material. 
/// </summary>
/// <param name="material"></param>
/// <param name="createInfo"></param>
void VkMaterial::createFromGenericMaterial(const Material& material, VkSamplerDescriptorSetCreateInfo createInfo)
{
	uint32_t textureCount = 0;

	// Lambda for texture creation
	VkContext& ctx = this->context;
	auto createTexture = [&](std::string textureStr, std::unique_ptr<VkTexture>& vkTexture) {
		if (!textureStr.empty())
		{
			vkTexture = std::make_unique<VkTexture>(textureStr, ctx);
			textureCount++;
		}
	};

	createTexture(material.diffuseTexture, diffuse);
	createTexture(material.specularTexture, specular);

	samplerDescriptorPool = createDescriptorPool(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		2,
		VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		context);

	createInfo.descriptorPool = samplerDescriptorPool;

	// Lambda for descriptor set creation
	auto& descriptors = samplerDescriptorSets;
	auto createDescriptor = [&](std::unique_ptr<VkTexture>& vkTexture) {
		if (vkTexture != nullptr)
		{
			VkDescriptorSet descriptorSet = vkTexture->createTextureSamplerDescriptor(createInfo);
			descriptors.push_back(descriptorSet);
		}
	};

	createDescriptor(diffuse);
	createDescriptor(specular);

	this->textureCount = textureCount;
}

void VkMaterial::apply()
{
	// NOT YET
}

void VkMaterial::cleanup()
{
	vkDestroyDescriptorPool(context.logicalDevice, samplerDescriptorPool, nullptr);
}

uint32_t VkMaterial::getTextureCount() const
{
	return textureCount;
}

const std::vector<VkDescriptorSet>& VkMaterial::getSamplerDescriptorSets() const
{
	return samplerDescriptorSets;
}
