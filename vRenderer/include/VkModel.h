#pragma once

#define GLFW_INCLUDE_VULKAN

#include <vector>
#include <map>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"
#include "VkMesh.h"
#include "VkMaterial.h"
#include "BaseCamera.h"
#include <functional>

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

	void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, BaseCamera* camera, std::function<void(int)> callback);

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