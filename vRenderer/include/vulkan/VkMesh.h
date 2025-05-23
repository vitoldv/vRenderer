#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include "VulkanUtils.h"
#include "Mesh.h"

using namespace VkUtils;

class VkMesh
{

public:

	const uint32_t id;

	VkMesh(uint32_t id, const Mesh& mesh, VkContext context);
	~VkMesh();

	int getVertexCount();
	VkBuffer getVertexBuffer();
	int getIndexCount();
	VkBuffer getIndexBuffer();
	glm::mat4 getTransformMat();

	void setTransformMat(glm::mat4 transform);

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

	void createFromGenericMesh(const Mesh& mesh);
	void createVertexBuffer(const std::vector<Vertex>& vertices, VkContext context);
	void createIndexBuffer(const std::vector<uint32_t>& indices, VkContext context);

	void cleanup();
};

