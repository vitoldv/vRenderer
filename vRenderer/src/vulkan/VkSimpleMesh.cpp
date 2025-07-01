#include "VkSimpleMesh.h"

std::array<glm::vec3, 8> cubeVertices = {
	glm::vec3(-1.0f, -1.0f, -1.0f),
	glm::vec3(1.0f, -1.0f, -1.0f),
	glm::vec3(1.0f,  1.0f, -1.0f),
	glm::vec3(-1.0f,  1.0f, -1.0f),
	glm::vec3(-1.0f, -1.0f,  1.0f),
	glm::vec3(1.0f, -1.0f,  1.0f),
	glm::vec3(1.0f,  1.0f,  1.0f),
	glm::vec3(-1.0f,  1.0f,  1.0f)
};

std::array<uint32_t, 36> cubeIndices = {
	0, 1, 2,  2, 3, 0,    // -Z
	5, 4, 7,  7, 6, 5,    // +Z
	4, 0, 3,  3, 7, 4,    // -X
	1, 5, 6,  6, 2, 1,    // +X
	3, 2, 6,  6, 7, 3,    // +Y
	4, 5, 1,  1, 0, 4     // -Y
};

VkSimpleMesh::VkSimpleMesh(uint32_t id, VK_PRIMITIVE_MESH type, VkContext context) :
	id(id)
{
	this->context = context;
	if (type == VK_PRIMITIVE_MESH::CUBE)
	{
		vertexCount = cubeVertices.size();
		indexCount = cubeIndices.size();
		createVertexBuffer(cubeVertices.data(), vertexCount, context);
		createIndexBuffer(cubeIndices.data(), indexCount, context);
	}
}

VkSimpleMesh::~VkSimpleMesh()
{
	//cleanup();
}

void VkSimpleMesh::cmdDraw(VkCommandBuffer commandBuffer)
{
	VkDeviceSize offsets[] = { 0 };																					// offsets into buffers being bound
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);								// Command to bind vertex buffer before deawing with them
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

int VkSimpleMesh::getVertexCount()
{
	return vertexCount;
}

VkBuffer VkSimpleMesh::getVertexBuffer()
{
	return vertexBuffer;
}

int VkSimpleMesh::getIndexCount()
{
	return indexCount;
}

VkBuffer VkSimpleMesh::getIndexBuffer()
{
	return indexBuffer;
}

glm::mat4 VkSimpleMesh::getTransformMat()
{
	return transformMat;
}

void VkSimpleMesh::setTransformMat(glm::mat4 transform)
{
	transformMat = transform;
}

void VkSimpleMesh::createVertexBuffer(glm::vec3* vertices, uint32_t count, VkContext context)
{
	// Size of buffer needed for vertices
	VkDeviceSize bufferSize = sizeof(glm::vec3) * count;

	// Temporary buffer to stage vertex data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(context.physicalDevice, context.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	// MAP MEMORY TO STAGE BUFFER
	void* data;
	vkMapMemory(context.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);		// "map" the vertex buffer memory to some point
	memcpy(data, vertices, (size_t)(bufferSize));									// copy memory from vertices std::vector to the point
	vkUnmapMemory(context.logicalDevice, stagingBufferMemory);										// unmap the vertex buffer memory

	// Create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also vertex buffer
	// Buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host))
	createBuffer(context.physicalDevice, context.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory);

	copyBuffer(context.logicalDevice, context.graphicsQueue, context.graphicsCommandPool, stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(context.logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(context.logicalDevice, stagingBufferMemory, nullptr);
}

void VkSimpleMesh::createIndexBuffer(uint32_t* indices, uint32_t count, VkContext context)
{
	// Size of buffer needed for indices
	VkDeviceSize bufferSize = sizeof(uint32_t) * count;

	// Temporary buffer to stage indices data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(context.physicalDevice, context.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	// MAP MEMORY TO STAGE BUFFER
	void* data;
	vkMapMemory(context.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);			// "map" the indices buffer memory to some point
	memcpy(data, indices, (size_t)(bufferSize));									// copy memory from indices std::vector to the point
	vkUnmapMemory(context.logicalDevice, stagingBufferMemory);								// unmap the indices buffer memory

	// Create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also indices buffer
	// Buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host))
	createBuffer(context.physicalDevice, context.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &indexBuffer, &indexBufferMemory);

	copyBuffer(context.logicalDevice, context.graphicsQueue, context.graphicsCommandPool, stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(context.logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(context.logicalDevice, stagingBufferMemory, nullptr);
}

void VkSimpleMesh::cleanup()
{
	vkDestroyBuffer(context.logicalDevice, indexBuffer, nullptr);
	vkFreeMemory(context.logicalDevice, indexBufferMemory, nullptr);

	vkDestroyBuffer(context.logicalDevice, vertexBuffer, nullptr);
	vkFreeMemory(context.logicalDevice, vertexBufferMemory, nullptr);
}