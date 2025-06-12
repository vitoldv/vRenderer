#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <numeric>
#include <iostream>

#include "ISceneInstanceTemplate.h"

#include "Mesh.h"
#include "Material.h"

/*
	Generic class for imported model
*/ 
class Model : public ISceneInstanceTemplate
{
public:
	//const std::string folderPath;
	const std::string name;

	Model(uint32_t id, std::string name, std::vector<std::unique_ptr<Mesh>>&& meshes, std::vector<std::unique_ptr<Material>>&& materials, uint32_t materialCount);
	virtual ~Model() = default;

	uint32_t getMeshCount() const;
	uint32_t getMaterialCount() const;
	std::string getName();

	const std::vector<std::unique_ptr<Mesh>>& getMeshes() const;
	const std::vector<std::unique_ptr<Material>>& getMaterials() const;

private:

	uint32_t meshCount;
	uint32_t materialCount;

	// Meshes of this model
	std::vector<std::unique_ptr<Mesh>> meshes;
	// Materials names applied to meshes of this model. Has a 1:1 relation to meshes std::vector.
	std::vector<std::unique_ptr<Material>> materials;
};