#pragma once

#define GLFW_INCLUDE_VULKAN

#include <vector>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"
#include "VkMesh.h"
#include "VkTexture.h"

using namespace VkUtils;

class VkModel
{
public:

	const uint32_t id;

	VkModel(uint32_t id, const Model& model, VkContext context, VkSamplerDescriptorSetCreateInfo createInfo);
	~VkModel();

	int getMeshCount() const;
	int getTextureCount() const;

	VkMesh* getMesh(uint32_t id) const;
	const std::vector<VkMesh*>& getMeshes() const;
	VkDescriptorSet getSamplerDescriptorSetForMesh(uint32_t id);

	void setTransform(glm::mat4 transform);

private:

	int meshCount;
	int textureCount;
	VkContext context;
	glm::mat4 transform;

	std::vector<VkMesh*> meshes;
	std::vector<VkTexture*> textures;

	VkDescriptorPool samplerDescriptorPool;
	std::vector<VkDescriptorSet> samplerDescriptorSets;

	void createFromGenericModel(const Model& model, VkSamplerDescriptorSetCreateInfo createInfo);
	void cleanup();
};