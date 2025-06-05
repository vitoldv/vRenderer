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

void VkMaterial::bind(uint32_t imageIndex, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	// Bind sampler descriptor set
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
		samplerDescriptorSetIndex, 1, &samplerDescriptorSet, 0, nullptr);

	componentsUniform->cmdBind(imageIndex, uniformDescriptorSetIndex, commandBuffer, pipelineLayout);
}

/// <summary>
/// Creates VkTexture along with sampler descriptor set for each specific texture type if one present in template material.
/// If texture type is not present - a corresponding null descriptor is created.
/// </summary>
/// <param name="material"></param>
/// <param name="createInfo"></param>
void VkMaterial::createFromGenericMaterial(const Material& material, VkSamplerDescriptorSetCreateInfo createInfo)
{
	uniformDescriptorPool = createDescriptorPool(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, context);

	components.ambientColor = glm::vec4(material.ambientColor, 1.0f);
	components.diffuseColor = glm::vec4(material.diffuseColor, 1.0f);
	components.specularColor = glm::vec4(material.specularColor, 1.0f);
	components.opacity = material.opacity;
	components.shininess = material.shininess;
	componentsUniform = std::make_unique<VkUniform<UboMaterial>>(
		3,		// IMAGE_COUNT
		uniformDescriptorSetIndex,
		uniformDescriptorPool,
		// TODO get rid of these embarassment
		VkSetLayoutFactory::instance().getSetLayout(DESC_SET_LAYOUT::MATERIAL_2),
		context);
	componentsUniform->updateAll(components);

	// SAMPLER DESCRIPTOR SET CREATION
	{
		// Create sampler pool
		samplerDescriptorPool = createDescriptorPool(
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			textureCount,
			VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			context);

		// Lambda for texture creation
		VkContext& ctx = this->context;
		auto createTexture = [&](std::string textureStr, std::unique_ptr<VkTexture>& vkTexture) {
			if (!textureStr.empty())
			{
				vkTexture = std::make_unique<VkTexture>(textureStr, ctx);
			}
			};

		createTexture(material.ambientTexture, ambient);
		createTexture(material.diffuseTexture, diffuse);
		createTexture(material.specularTexture, specular);
		createTexture(material.opacityMap, opacityMap);

		createSamplerDescriptorSet(createInfo);
	}
}

/// <summary>
/// Creates a texture sampler descriptor set.
/// If dummy flag is set, an empty descriptor (allocated but not bound to any resource) is returned.
/// </summary>
/// <param name="createInfo"></param>
/// <param name="dummy"></param>
/// <returns></returns>
void VkMaterial::createSamplerDescriptorSet(VkSamplerDescriptorSetCreateInfo createInfo)
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	VkDescriptorSetLayout samplerSetLayout = VkSetLayoutFactory::instance().getSetLayout(DESC_SET_LAYOUT::MATERIAL);
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = samplerDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &samplerSetLayout;

	VkResult result = vkAllocateDescriptorSets(context.logicalDevice, &allocInfo, &samplerDescriptorSet);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate texture sampler descriptor sets.");
	}

	std::array<VkWriteDescriptorSet, 4> setWrites;
	std::array<VkDescriptorImageInfo, 4> imageInfos;	// required for avoiding lifetime issues
	auto createDescriptorForTexture = [&](const VkTexture* texture, uint32_t binding) {
		
		// If texture exists - we bind its imageView an descriptor's resource
		if (texture != nullptr)
		{
			// texture image info
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texture->getImageView();
			imageInfo.sampler = createInfo.sampler;
			imageInfos[binding] = imageInfo;
		}
		// If there is no texture - we create dummy (empty) image as descriptor's resource
		// nullDescriptor device feature should be enabled for this to work
		else
		{
			uint32_t dummyBufferSize = 16;
			VkBuffer dummyBuffer;
			VkDeviceMemory dummyBufferMemory;
			VkUtils::createBuffer(context.physicalDevice, context.logicalDevice, dummyBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &dummyBuffer, &dummyBufferMemory);
			dummyBuffers.push_back(dummyBuffer);
			dummyBuffersMemory.push_back(dummyBufferMemory);

			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.imageView = VK_NULL_HANDLE;
			imageInfo.sampler = createInfo.sampler;
			imageInfos[binding] = imageInfo;
		}
		
		VkWriteDescriptorSet setWrite = {};
		setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		setWrite.descriptorCount = 1;
		setWrite.dstSet = samplerDescriptorSet;
		setWrite.dstArrayElement = 0;
		setWrite.dstBinding = binding;
		setWrite.pImageInfo = &imageInfos[binding];
		setWrites[binding] = setWrite;
	};

	// Passed bindings should match the ones in shader (as well as order of calling here)
	createDescriptorForTexture(ambient.get(), 0);
	createDescriptorForTexture(diffuse.get(), 1);
	createDescriptorForTexture(specular.get(), 2);
	createDescriptorForTexture(opacityMap.get(), 3);

	vkUpdateDescriptorSets(context.logicalDevice, textureCount, setWrites.data(), 0, nullptr);
}

void VkMaterial::cleanup()
{
	componentsUniform->cleanup();
	vkDestroyDescriptorPool(context.logicalDevice, uniformDescriptorPool, nullptr);
	vkDestroyDescriptorPool(context.logicalDevice, samplerDescriptorPool, nullptr);
	for (int i = 0; i < dummyBuffers.size(); i++)
	{
		vkDestroyBuffer(context.logicalDevice, dummyBuffers[i], nullptr);
		vkFreeMemory(context.logicalDevice, dummyBuffersMemory[i], nullptr);
	}
}
