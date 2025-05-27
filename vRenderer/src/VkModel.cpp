#include "VkModel.h"

VkModel::VkModel(uint32_t id, const Model& model, VkContext context, VkSamplerDescriptorSetCreateInfo createInfo) :
	id(id)
{
	this->context = context;

	meshCount = model.getMeshCount();
	meshes.resize(meshCount);
	materials.resize(meshCount);

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

int VkModel::getMaterialCount() const
{
	return materialCount;
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
	for (int i = 0; i < meshCount; i++)
	{
		auto& mesh = meshes[i];

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

		bool textured = materials[i] != nullptr && materials[i]->getTextureCount() > 0;

		// PUSH CONSTANTS
		{
			PushConstant push = {};
			push.model = mesh->getTransformMat();
			push.useTexture = textured;
			vkCmdPushConstants(commandBuffer, pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), &push);
		}

		if (textured)
		{
			auto* material = materials[i];
			
			// Uniform descriptor set
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
				0, 1, &descriptorSet, 0, nullptr);
			// Material sampler descriptor sets
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
				1, material->getTextureCount(), material->getSamplerDescriptorSets().data(), 0, nullptr);
		}
		else
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
				0, 1, &descriptorSet, 0, nullptr);
		}

		// execute pipeline
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh->getIndexCount()), 1, 0, -VERTEX_INDEX_OFFSET, 0);
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
	for (int i = 0; i < model.getMeshCount(); i++)
	{
		const Mesh& mesh = model.getMeshes()[i];
		
		uint32_t newMeshId = i;
		VkMesh* vkMesh = new VkMesh(newMeshId, mesh, context);
		
		auto material = model.getMaterials()[i];
		VkMaterial* vkMaterial = nullptr;
		if (material != nullptr)
		{
			materialCount++;
			vkMaterial = new VkMaterial(*material, context, createInfo);
		}

		meshes[i] = vkMesh;
		materials[i] = vkMaterial;
	}
}

void VkModel::cleanup()
{
	for (auto& texture : materials)
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
