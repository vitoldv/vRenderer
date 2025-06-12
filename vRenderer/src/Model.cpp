#include "Model.h"
#include "utils.h"

Model::Model(uint32_t id, std::string name, std::vector<std::unique_ptr<Mesh>>&& meshes, std::vector<std::unique_ptr<Material>>&& materials, uint32_t materialCount) :
	ISceneInstanceTemplate(id, name)	
{
	this->meshCount = meshes.size();
	this->meshes = std::move(meshes);
	this->materials = std::move(materials);
	this->materialCount = materialCount;
}

uint32_t Model::getMeshCount() const
{
	return this->meshCount;
}

uint32_t Model::getMaterialCount() const
{
	return this->materialCount;
}

std::string Model::getName()
{
	return this->name;
}

const std::vector<std::unique_ptr<Mesh>>& Model::getMeshes() const
{
	return this->meshes;
}

const std::vector<std::unique_ptr<Material>>& Model::getMaterials() const
{
	return this->materials;
}