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

void VkModel::draw(uint32_t imageIndex, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, const BaseCamera& camera)
{
	for (int i = 0; i < meshCount; i++)
	{
		auto& mesh = meshes[i];

		VkBuffer vertexBuffers[] = { mesh->getVertexBuffer() };															// buffers to bind
		VkBuffer indexBuffer = mesh->getIndexBuffer();
		VkDeviceSize offsets[] = { 0 };																					// offsets into buffers being bound
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);								// Command to bind vertex buffer before deawing with them
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// PUSH CONSTANTS
		{
			PushConstant push = {};
			push.model = mesh->getTransformMat();
			push.normalMatrix = glm::transpose(glm::inverse(mesh->getTransformMat()));
			push.viewPosition = camera.getPosition();
			vkCmdPushConstants(commandBuffer, pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), &push);
		}

		auto* material = materials[i];
		// Material sampler uniforms
		material->cmdBind(imageIndex, commandBuffer, pipelineLayout);


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
