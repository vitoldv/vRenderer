#pragma once

#include <string>
#include <memory>

#include "Material.h"
#include "VkTexture.h"

class VkMaterial
{
public:

	const std::string name;

	uint32_t textureCount;
	std::unique_ptr<VkTexture> diffuse;
	std::unique_ptr<VkTexture> specular;

	VkMaterial(const Material& material, VkContext context, VkSamplerDescriptorSetCreateInfo createInfo);
	~VkMaterial();

	void apply();
	void cleanup();

	uint32_t getTextureCount() const;
	const std::vector<VkDescriptorSet>& getSamplerDescriptorSets() const;

private:

	// Color applied to fragment if diffuse texture is missing
	const glm::vec3 diffuseColor = { 1.0f, 0.5f, 0.31f };

	VkContext context;
	VkDescriptorPool samplerDescriptorPool;
	std::vector<VkDescriptorSet> samplerDescriptorSets;

	void createFromGenericMaterial(const Material& material, VkSamplerDescriptorSetCreateInfo createInfo);
};