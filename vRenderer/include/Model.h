#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <numeric>
#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "ISceneInstanceTemplate.h"

#include "geometry_settings.h"
#include "Mesh.h"
#include "Material.h"

/*
	Generic class for imported model
*/ 
class Model : public ISceneInstanceTemplate
{
public:
	const std::string folderPath;
	const std::string name;

	Model(uint32_t id, std::string filePath);
	virtual ~Model() = default;

	uint32_t getMeshCount() const;
	uint32_t getMaterialCount() const;
	std::string getName();

	const std::vector<Mesh>& getMeshes() const;
	const std::vector<Material*>& getMaterials() const;

private:

	uint32_t meshCount;
	uint32_t materialsCount;

	// Meshes of this model
	std::vector<Mesh> meshes;
	// Materials names applied to meshes of this model. Has a 1:1 relation to meshes std::vector.
	std::vector<Material*> materials;

	void importModel(std::string filePath);
};