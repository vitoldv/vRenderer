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

VkMesh* VkModel::getMesh(uint32_t id) const
{
	auto it = std::find_if(meshes.begin(), meshes.end(), [id](VkMesh* mesh) {return mesh->id == id;});
	return *it;
}

const std::vector<VkMesh*>& VkModel::getMeshes() const
{
	return meshes;
}

VkDescriptorSet VkModel::getSamplerDescriptorSetForMesh(uint32_t meshId)
{
	VkMesh* mesh = getMesh(meshId);
	if (mesh->hasTexture())
	{
		return samplerDescriptorSets[mesh->getTextureIndex()];
	}
	return {};
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
		VkMesh* vkMesh = new VkMesh(i, mesh, context);
		auto textureName = model.getTextures()[i];
		VkTexture* vkTexture = nullptr;
		if (!textureName.empty())
		{
			textureCount++;
			vkTexture = new VkTexture(model.getFullTexturePath(i), context);
			VkDescriptorSet descriptorSet = vkTexture->createTextureSamplerDescriptor(createInfo);
			samplerDescriptorSets.push_back(descriptorSet);
			vkMesh->setTextureDescriptorIndex(samplerDescriptorSets.size() - 1);
		}
		else
		{
			vkMesh->setTextureDescriptorIndex(-1);
		}

		meshes[i] = vkMesh;
		textures[i] = vkTexture;
	}
}

void VkModel::cleanup()
{
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
