#include "VkMesh.h"

VkMesh::VkMesh(uint32_t id, const Mesh& mesh, VkContext context) : 
	id(id)
{
	this->context = context;
	this->textureIndex = textureIndex;
	createFromGenericMesh(mesh);
}

VkMesh::~VkMesh()
{
	cleanup();
}

bool VkMesh::hasTexture()
{
	return textureIndex > -1;
}

int VkMesh::getVertexCount()
{
	return this->vertexCount;
}

VkBuffer VkMesh::getVertexBuffer()
{
	return this->vertexBuffer;
}

int VkMesh::getIndexCount()
{
	return this->indexCount;
}

VkBuffer VkMesh::getIndexBuffer()
{
	return this->indexBuffer;
}

int VkMesh::getTextureIndex()
{
	return this->textureIndex;
}

glm::mat4 VkMesh::getTransformMat()
{
	return this->transformMat;
}

void VkMesh::setTransformMat(glm::mat4 transform)
{
	this->transformMat = transform;
}

void VkMesh::setTextureDescriptorIndex(int textureDescriptorIndex)
{
	this->textureIndex = textureDescriptorIndex;
}

void VkMesh::createFromGenericMesh(const Mesh& mesh)
{
	std::vector<Vertex> vertices;
	const auto& meshVertices = mesh.getVertices();
	const auto& meshIndices = mesh.getIndices();
	const auto& meshTexCoords = mesh.getTexCoords();
	const auto& meshNormals = mesh.getNormals();
	for (int i = 0; i < meshVertices.size(); i++)
	{
		Vertex vertex = {};
		vertex.pos = meshVertices.at(i);
		vertex.normal = meshNormals.at(i);
		vertex.uv = meshTexCoords.at(i);
		vertices.push_back(vertex);
	}

	this->indexCount = meshIndices.size();
	this->vertexCount = vertices.size();

	setTransformMat(glm::identity<glm::mat4>());

	createVertexBuffer(vertices, context);
	createIndexBuffer(meshIndices, context);
}

void VkMesh::createVertexBuffer(const std::vector<Vertex>& vertices, VkContext context)
{
	// Size of buffer needed for vertices
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

	// Temporary buffer to stage vertex data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(context.physicalDevice, context.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	// MAP MEMORY TO STAGE BUFFER
	void* data;
	vkMapMemory(context.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);		// "map" the vertex buffer memory to some point
	memcpy(data, vertices.data(), (size_t)(bufferSize));									// copy memory from vertices std::vector to the point
	vkUnmapMemory(context.logicalDevice, stagingBufferMemory);										// unmap the vertex buffer memory

	// Create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also vertex buffer
	// Buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host))
	createBuffer(context.physicalDevice, context.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory);

	copyBuffer(context.logicalDevice, context.graphicsQueue, context.graphicsCommandPool, stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(context.logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(context.logicalDevice, stagingBufferMemory, nullptr);
}

void VkMesh::createIndexBuffer(const std::vector<uint32_t>& indices, VkContext context)
{
	// Size of buffer needed for indices
	VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();

	// Temporary buffer to stage indices data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(context.physicalDevice, context.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	// MAP MEMORY TO STAGE BUFFER
	void* data;
	vkMapMemory(context.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);			// "map" the indices buffer memory to some point
	memcpy(data, indices.data(), (size_t)(bufferSize));									// copy memory from indices std::vector to the point
	vkUnmapMemory(context.logicalDevice, stagingBufferMemory);								// unmap the indices buffer memory

	// Create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also indices buffer
	// Buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host))
	createBuffer(context.physicalDevice, context.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &this->indexBuffer, &this->indexBufferMemory);

	copyBuffer(context.logicalDevice, context.graphicsQueue, context.graphicsCommandPool, stagingBuffer, this->indexBuffer, bufferSize);

	vkDestroyBuffer(context.logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(context.logicalDevice, stagingBufferMemory, nullptr);
}

void VkMesh::cleanup()
{
	vkDestroyBuffer(context.logicalDevice, this->indexBuffer, nullptr);
	vkFreeMemory(context.logicalDevice, this->indexBufferMemory, nullptr);

	vkDestroyBuffer(context.logicalDevice, this->vertexBuffer, nullptr);
	vkFreeMemory(context.logicalDevice, this->vertexBufferMemory, nullptr);
}
