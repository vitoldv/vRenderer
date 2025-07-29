#pragma once

#define GLFW_INCLUDE_VULKAN

#include <vector>
#include <map>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

#include "geometry_settings.h"
#include "Model.h"
#include "VkMaterial.h"
#include "BaseCamera.h"
#include "IVkCoreResourceHolder.h"

using namespace VkUtils;

class VkModel : IVkCoreResourceHolder
{

public:
	const uint32_t id;

	VkModel(uint32_t id, const Model& model, VkContext context, VkSamplerDescriptorSetCreateInfo createInfo);
	~VkModel();

	int getMeshCount() const;
	int getMaterialCount() const;
	int getMeshIndexCount(uint32_t meshIndex) const;

	void cmdBindSubMesh(uint32_t imageIndex, uint32_t submeshIndex, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, bool bindMaterials);
	void cmdDraw(uint32_t imageIndex, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, bool bindMaterials);
	void setTransform(glm::mat4 transform);

private:

	const uint32_t NO_MATERIAL_INDEX = -1;

	struct VkSubMesh
	{
		uint32_t id;
		uint32_t vertexCount;
		uint32_t indexCount;
		VkDeviceSize vertexBufferOffset;
		VkDeviceSize indexBufferOffset;
		uint32_t materialIndex;
	};

	glm::mat4 transform;

	std::vector<VkSubMesh> meshes;
	std::vector<std::unique_ptr<VkMaterial>> materials;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	VkContext context;

	void createFromModel(const Model& model, VkSamplerDescriptorSetCreateInfo createInfo);
	void createVertexBuffer(const std::vector<std::vector<VkUtils::Vertex>*>& vertexBuffers, VkContext context);
	void createIndexBuffer(const std::vector<const std::vector<uint32_t>*>& indexBuffers, VkContext context);

	void cleanup();
};