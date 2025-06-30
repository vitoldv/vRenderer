#pragma once

#include "VulkanUtils.h"
#include "geometry_settings.h"

using namespace VkUtils;

enum class VK_PRIMITIVE_MESH
{
	CUBE
};

/// <summary>
/// Represents a mesh of position-only vertices. Intended to be used
/// as a primitive render target.
/// </summary>
class VkSimpleMesh
{

public:

	const uint32_t id;

	VkSimpleMesh(uint32_t id, VK_PRIMITIVE_MESH type, VkContext context);
	~VkSimpleMesh();

	void cmdDraw(VkCommandBuffer commandBuffer);

	int getVertexCount();
	VkBuffer getVertexBuffer();
	int getIndexCount();
	VkBuffer getIndexBuffer();
	glm::mat4 getTransformMat();

	void setTransformMat(glm::mat4 transform);
		void cleanup();

private:

	int vertexCount;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	int indexCount;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	VkContext context;

	// Transform
	glm::mat4 transformMat;

	void createVertexBuffer(glm::vec3* vertices, uint32_t count, VkContext context);
	void createIndexBuffer(uint32_t* indices, uint32_t count, VkContext context);

};
