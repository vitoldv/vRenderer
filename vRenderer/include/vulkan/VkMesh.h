#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include "VulkanUtils.h"
#include "Mesh.h"

using namespace VkUtils;

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 normal;
	glm::vec2 uv;
};

class VkMesh
{

public:

	const uint32_t id;

	VkMesh(uint32_t id, const Mesh& mesh, VkContext context);
	~VkMesh();

	bool hasTexture();
	int getVertexCount();
	VkBuffer getVertexBuffer();
	int getIndexCount();
	VkBuffer getIndexBuffer();
	int getTextureIndex();
	glm::mat4 getTransformMat();

	void setTransformMat(glm::mat4 transform);
	void setTextureDescriptorIndex(int textureDescriptorIndex);

private:

	int vertexCount;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;

	int indexCount;
	VkBuffer indexBuffer; 
	VkDeviceMemory indexBufferMemory;

	int textureIndex;

	VkContext context;

	// Transform
	glm::mat4 transformMat;

	void createFromGenericMesh(const Mesh& mesh);
	void createVertexBuffer(const std::vector<Vertex>& vertices, VkContext context);
	void createIndexBuffer(const std::vector<uint32_t>& indices, VkContext context);

	void cleanup();
};

