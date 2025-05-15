#include "VkModel.h"

VkModel::VkModel(uint32_t id, const Model& model, VkContext context, VkSamplerDescriptorSetCreateInfo createInfo) :
	id(id)
{
	this->context = context;

	meshCount = model.getMeshCount();
	meshes.resize(meshCount);
	textures.resize(meshCount);

	createFromGenericModel(model, createInfo);
}

VkModel::~VkModel()
{
	cleanup();
}

int VkModel::getMeshCount() const
{
	return meshCount;
}

int VkModel::getTextureCount() const
{
	return textureCount;
}

const VkMesh* VkModel::getMesh(uint32_t id) const
{
	auto it = std::find_if(meshes.begin(), meshes.end(), [id](VkMesh* mesh) {return mesh->id == id;});
	return *it;
}

const std::vector<VkMesh*>& VkModel::getMeshes() const
{
	return meshes;
}

void VkModel::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet)
{
	for (auto& mesh : meshes)
	{
		VkBuffer vertexBuffers[] = { mesh->getVertexBuffer() };															// buffers to bind
		VkBuffer indexBuffer = mesh->getIndexBuffer();
		VkDeviceSize offsets[] = { 0 };																					// offsets into buffers being bound
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);								// Command to bind vertex buffer before deawing with them
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// LEFT FOR REFERENCE ON DYNAMIC UNIFORM BUFFERS
		//// Dynamic offset amount
		//uint32_t dynamicOffset = static_cast<uint32_t>(modelUniformAlignment) * meshCount;
		//vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->vkPipelineLayout,
		//	0, 1, &this->vkDescriptorSets[currentImage], 1, &dynamicOffset);

		uint32_t descriptorIndex = meshSamplerDescriptorMap[mesh->id];

		bool textured = descriptorIndex != NO_TEXTURE_INDEX;

		PushConstant push = {};
		push.model = mesh->getTransformMat();
		push.useTexture = textured;
		vkCmdPushConstants(commandBuffer, pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), &push);

		if (textured)
		{
			std::array<VkDescriptorSet, 2> descriptorSets = { descriptorSet, samplerDescriptorSets[descriptorIndex]};

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
				0, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(), 0, nullptr);
		}
		else
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
				0, 1, &descriptorSet, 0, nullptr);
		}

		// execute pipeline
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh->getIndexCount()), 1, 0, -VERTEX_INDEX_OFFSET, 0);

		meshCount++;
	}
}

void VkModel::setTransform(glm::mat4 transform)
{
	for (auto* mesh : meshes)
	{
		mesh->setTransformMat(transform);
	}
}

void VkModel::createFromGenericModel(const Model& model, VkSamplerDescriptorSetCreateInfo createInfo)
{
	samplerDescriptorPool = createDescriptorPool(
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		model.getTextureCount(),
		VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		context);

	createInfo.descriptorPool = samplerDescriptorPool;

	for (int i = 0; i < model.getMeshCount(); i++)
	{
		const Mesh& mesh = model.getMeshes()[i];
		
		uint32_t newMeshId = i;
		VkMesh* vkMesh = new VkMesh(newMeshId, mesh, context);
		
		auto textureName = model.getTextures()[i];
		VkTexture* vkTexture = nullptr;
		if (!textureName.empty())
		{
			textureCount++;
			vkTexture = new VkTexture(model.getFullTexturePath(i), context);
			
			VkDescriptorSet descriptorSet = vkTexture->createTextureSamplerDescriptor(createInfo);
			samplerDescriptorSets.push_back(descriptorSet);
			
			meshSamplerDescriptorMap[newMeshId] = samplerDescriptorSets.size() - 1;
		}
		else
		{
			meshSamplerDescriptorMap[newMeshId] = NO_TEXTURE_INDEX;
		}

		meshes[i] = vkMesh;
		textures[i] = vkTexture;
	}
}

void VkModel::cleanup()
{
	vkDestroyDescriptorPool(context.logicalDevice, samplerDescriptorPool, nullptr);

	for (auto& texture : textures)
	{
		delete texture;
		texture = nullptr;
	}
	for (auto& mesh : meshes)
	{
		delete mesh;
		mesh = nullptr;
	}
}
