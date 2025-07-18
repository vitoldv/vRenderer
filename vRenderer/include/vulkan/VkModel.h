#pragma once

#define GLFW_INCLUDE_VULKAN

#include <vector>
#include <map>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

#include "geometry_settings.h"
#include "Model.h"
#include "VkMesh.h"
#include "VkMaterial.h"
#include "BaseCamera.h"

using namespace VkUtils;

class VkModel
{
public:

	const uint32_t id;

	VkModel(uint32_t id, const Model& model, VkContext context, VkSamplerDescriptorSetCreateInfo createInfo);
	~VkModel();

	int getMeshCount() const;
	int getMaterialCount() const;

	const VkMesh* getMesh(uint32_t id) const;
	const std::vector<VkMesh*>& getMeshes() const;

	void draw(uint32_t imageIndex, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, bool bindMaterials);

	void setTransform(glm::mat4 transform);

private:

	const uint32_t NO_MATERIAL_INDEX = -1;

	int meshCount;
	int materialCount;
	VkContext context;
	glm::mat4 transform;

	// 1:1 relation
	std::vector<VkMesh*> meshes;
	std::vector<VkMaterial*> materials;

	void createFromGenericModel(const Model& model, VkSamplerDescriptorSetCreateInfo createInfo);
	void cleanup();
};