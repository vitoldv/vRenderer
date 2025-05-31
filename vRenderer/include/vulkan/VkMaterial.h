#pragma once

#include <string>
#include <memory>

#include "Material.h"
#include "VkTexture.h"

class VkMaterial
{
public:

	const std::string name;	
	const uint32_t textureCount = 2;
	const uint32_t descriptorSetIndex = 1;

	// The order of texture creation affects the order of sampler descriptors in samplerDescriptorSets vector,
	// and, accordingly, what texture is passed in shader.
	// This way, let the order of declaration here match the order of creation and the declaration order in shader.
	std::unique_ptr<VkTexture> diffuse;
	std::unique_ptr<VkTexture> specular;

	VkMaterial(const Material& material, VkContext context, VkSamplerDescriptorSetCreateInfo createInfo);
	~VkMaterial();

	void cleanup();

	const VkDescriptorSet& getSamplerDescriptorSet() const;

private:

	VkContext context;
	VkDescriptorPool samplerDescriptorPool;
	VkDescriptorSet samplerDescriptorSet;

	// Storage for dummy buffers and their memory used for null descriptors initialization
	std::vector<VkBuffer> dummyBuffers;
	std::vector<VkDeviceMemory> dummyBuffersMemory;
	
	void createFromGenericMaterial(const Material& material, VkSamplerDescriptorSetCreateInfo createInfo);
	void createSamplerDescriptorSet(VkSamplerDescriptorSetCreateInfo createInfo);
};