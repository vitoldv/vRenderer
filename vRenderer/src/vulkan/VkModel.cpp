#include "VkModel.h"

VkModel::VkModel(uint32_t id, const Model& model, VkContext context, VkSamplerDescriptorSetCreateInfo createInfo) :
	id(id)
{
	this->context = context;
	this->transform = glm::mat4(1.0f);

	createFromModel(model, createInfo);
}

VkModel::~VkModel()
{
	cleanup();
}

int VkModel::getMeshCount() const
{
	return meshes.size();
}

int VkModel::getMaterialCount() const
{
	return materials.size();
}

void VkModel::draw(uint32_t imageIndex, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, bool bindMaterials)
{	
	for (const VkSubMesh& mesh : meshes)
	{
		uint32_t indexCount = mesh.indexCount;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &mesh.vertexBufferOffset);								// Command to bind vertex buffer before deawing with them
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, mesh.indexBufferOffset, VK_INDEX_TYPE_UINT32);

		// PUSH CONSTANTS
		{
			PushConstant push = {};
			push.model = this->transform;
			push.normalMatrix = glm::transpose(glm::inverse(this->transform));
			vkCmdPushConstants(commandBuffer, pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), &push);
		}

		if (bindMaterials)
		{
			materials[mesh.materialIndex]->cmdBind(imageIndex, commandBuffer, pipelineLayout);
		}

		// execute pipeline
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, -VERTEX_INDEX_OFFSET, 0);
	}
}

void VkModel::setTransform(glm::mat4 transform)
{
	this->transform = transform;
}

void VkModel::createFromModel(const Model& model, VkSamplerDescriptorSetCreateInfo createInfo)
{
	uint32_t meshCount = model.getMeshCount();
	meshes.resize(meshCount);

	std::vector<std::vector<Vertex>*> vertexBuffers(meshCount);
	std::vector<const std::vector<uint32_t>*> indexBuffers(meshCount);

	uint32_t currentSubMeshVertexOffset = 0;
	uint32_t currentSubMeshIndexOffset = 0;

	for (int i = 0; i < meshCount; ++i)
	{
		const Mesh& mesh = *model.getMeshes()[i];

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

		uint32_t indexCount = meshIndices.size();
		uint32_t vertexCount = vertices.size();

		VkSubMesh subMesh = {
			i, vertexCount, indexCount,
			currentSubMeshVertexOffset,
			currentSubMeshIndexOffset
		};

		currentSubMeshVertexOffset += sizeof(Vertex) * vertexCount;
		currentSubMeshIndexOffset += sizeof(uint32_t) * indexCount;

		const auto& material = model.getMaterials()[i];
		if (material != nullptr)
		{
			materials.push_back(std::make_unique<VkMaterial>(*material, context, createInfo));
			subMesh.materialIndex = materials.size() - 1;
		}
		else
		{
			subMesh.materialIndex = NO_MATERIAL_INDEX;
		}
		

		meshes[i] = subMesh;

		vertexBuffers[i] = new std::vector<Vertex>(std::move(vertices));
		indexBuffers[i] = &meshIndices;
	}

	createVertexBuffer(vertexBuffers, context);
	createIndexBuffer(indexBuffers, context);
}

void VkModel::createVertexBuffer(const std::vector<std::vector<Vertex>*>& vertexBuffers, VkContext context)
{
	// Size of buffer needed for all vertex buffers
	VkDeviceSize bufferSize = 0;
	for (const auto& vertexBuffer : vertexBuffers)
	{
		bufferSize += sizeof(Vertex) * vertexBuffer->size();
	}

	// Temporary buffer to stage vertex data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(context.physicalDevice, context.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	// MAP MEMORY TO STAGE BUFFER
	void* data;
	vkMapMemory(context.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);		// "map" the vertex buffer memory to some point
	
	for (int i = 0; i < vertexBuffers.size(); ++i)
	{
		size_t dataSize = (size_t)(sizeof(Vertex) * vertexBuffers[i]->size());
		memcpy(data, vertexBuffers[i]->data(), dataSize);									// copy memory from vertices std::vector to the point
		data = static_cast<void*>((uint8_t*)data + dataSize);
	}

	vkUnmapMemory(context.logicalDevice, stagingBufferMemory);										// unmap the vertex buffer memory

	// Create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also vertex buffer
	// Buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host))
	createBuffer(context.physicalDevice, context.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vertexBuffer, &vertexBufferMemory);

	copyBuffer(context.logicalDevice, context.graphicsQueue, context.graphicsCommandPool, stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(context.logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(context.logicalDevice, stagingBufferMemory, nullptr);
}

void VkModel::createIndexBuffer(const std::vector<const std::vector<uint32_t>*>& indexBuffers, VkContext context)
{
	// Size of buffer needed for indices
	VkDeviceSize bufferSize = 0;
	for (const auto& indexBuffer : indexBuffers)
	{
		bufferSize += sizeof(uint32_t) * indexBuffer->size();
	}

	// Temporary buffer to stage indices data before transferring to GPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(context.physicalDevice, context.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&stagingBuffer, &stagingBufferMemory);

	// MAP MEMORY TO STAGE BUFFER
	void* data;
	vkMapMemory(context.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);			// "map" the indices buffer memory to some point
	for (int i = 0; i < indexBuffers.size(); ++i)
	{
		size_t dataSize = (size_t)(sizeof(uint32_t) * indexBuffers[i]->size());
		memcpy(data, indexBuffers[i]->data(), dataSize);									// copy memory from vertices std::vector to the point
		data = static_cast<void*>((uint8_t*)data + dataSize);
	}
	vkUnmapMemory(context.logicalDevice, stagingBufferMemory);								// unmap the indices buffer memory

	// Create buffer with TRANSFER_DST_BIT to mark as recipient of transfer data (also indices buffer
	// Buffer memory is to be DEVICE_LOCAL_BIT meaning memory is on the GPU and only accessible by it and not CPU (host))
	createBuffer(context.physicalDevice, context.logicalDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &indexBuffer, &vertexBufferMemory);

	copyBuffer(context.logicalDevice, context.graphicsQueue, context.graphicsCommandPool, stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(context.logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(context.logicalDevice, stagingBufferMemory, nullptr);
}

void VkModel::cleanup()
{
	vkDestroyBuffer(context.logicalDevice, indexBuffer, nullptr);
	vkFreeMemory(context.logicalDevice, indexBufferMemory, nullptr);
	vkDestroyBuffer(context.logicalDevice, vertexBuffer, nullptr);
	vkFreeMemory(context.logicalDevice, vertexBufferMemory, nullptr);
}
