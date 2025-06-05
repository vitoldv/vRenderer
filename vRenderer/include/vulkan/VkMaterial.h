#pragma once

#include <string>
#include <memory>

#include "Material.h"
#include "VkTexture.h"
#include "VkUniform.hpp"
#include "VulkanUtils.h"
#include "VkSetLayoutFactory.h"

class VkMaterial
{
public:

	const std::string name;	

	// Should match the amount of textures declared below
	static const uint32_t textureCount = 4;

	// The order of texture creation affects the order of sampler descriptors in samplerDescriptorSets vector,
	// and, accordingly, what texture is passed in shader.
	// This way, let the order of declaration here match the order of creation and the declaration order in shader.
	std::unique_ptr<VkTexture> ambient;
	std::unique_ptr<VkTexture> diffuse;
	std::unique_ptr<VkTexture> specular;
	std::unique_ptr<VkTexture> opacityMap;

	struct ALIGN_STD140 UboMaterial
	{
		glm::vec4 ambientColor;
		glm::vec4 diffuseColor;
		glm::vec4 specularColor;
		float opacity;
		float shininess;
	};

	UboMaterial components;

	VkMaterial(const Material& material, VkContext context, VkSamplerDescriptorSetCreateInfo createInfo);
	~VkMaterial();

	void cleanup();

	void cmdBind(uint32_t imageIndex, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);

private:

	VkContext context;
	VkDescriptorPool samplerDescriptorPool;
	VkDescriptorPool uniformDescriptorPool;
	VkDescriptorSet samplerDescriptorSet;

	std::unique_ptr<VkUniform<UboMaterial>> componentsUniform;

	uint32_t samplerDescriptorSetIndex;
	uint32_t uniformDescriptorSetIndex;

	// Storage for dummy buffers and their memory used for null descriptors initialization
	std::vector<VkBuffer> dummyBuffers;
	std::vector<VkDeviceMemory> dummyBuffersMemory;
	
	void createFromGenericMaterial(const Material& material, VkSamplerDescriptorSetCreateInfo createInfo);
	void createSamplerDescriptorSet(VkSamplerDescriptorSetCreateInfo createInfo);
};