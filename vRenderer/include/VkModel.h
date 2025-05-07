#pragma once

#define GLFW_INCLUDE_VULKAN

#include <vector>
#include <map>
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

	const VkMesh* getMesh(uint32_t id) const;
	const std::vector<VkMesh*>& getMeshes() const;

	void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet);

	void setTransform(glm::mat4 transform);

private:

	const uint32_t NO_TEXTURE_INDEX = -1;

	int meshCount;
	int textureCount;
	VkContext context;
	glm::mat4 transform;

	std::vector<VkMesh*> meshes;
	std::vector<VkTexture*> textures;
	
	// Key - mesh id; Value - index of corresponding texture descriptor in samplerDescriptorSets vector.
	// If mesh has no texture - value is -1.
	std::map<uint32_t, uint32_t> meshSamplerDescriptorMap;

	VkDescriptorPool samplerDescriptorPool;
	std::vector<VkDescriptorSet> samplerDescriptorSets;

	void createFromGenericModel(const Model& model, VkSamplerDescriptorSetCreateInfo createInfo);
	void cleanup();
};