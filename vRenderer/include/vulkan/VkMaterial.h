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

	// The order of texture creation affects the order of sampler descriptors in samplerDescriptorSets vector,
	// and, accordingly, what texture is passed in shader.
	// This way, let the order of declaration here match the order of creation and the declaration order in shader.
	std::unique_ptr<VkTexture> diffuse;
	std::unique_ptr<VkTexture> specular;

	VkMaterial(const Material& material, VkContext context, VkSamplerDescriptorSetCreateInfo createInfo);
	~VkMaterial();

	void cleanup();

	const std::vector<VkDescriptorSet>& getSamplerDescriptorSets() const;

private:

	VkContext context;
	VkDescriptorPool samplerDescriptorPool;
	std::vector<VkDescriptorSet> samplerDescriptorSets;

	// Storage for dummy buffers and their memory used for null descriptors initialization
	std::vector<VkBuffer> dummyBuffers;
	std::vector<VkDeviceMemory> dummyBuffersMemory;
	
	void createFromGenericMaterial(const Material& material, VkSamplerDescriptorSetCreateInfo createInfo);
	VkDescriptorSet createNullSamplerDescriptor(VkSamplerDescriptorSetCreateInfo createInfo);
};