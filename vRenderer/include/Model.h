#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <geometry_settings.h>
#include <Mesh.h>

/*
	Generic class for imported model
*/ 

class Model
{
public:
	const uint32_t id;
	const std::string folderPath;

	Model(uint32_t id, std::string filePath);
	~Model() = default;

	uint32_t getMeshCount() const;
	std::string getName();

	const std::vector<Mesh>& getMeshes() const;
	const std::vector<std::string>& getTextures() const;
	std::string getFullTexturePath(int textureIndex) const;

private:

	uint32_t meshCount;
	std::string name;

	// Meshes of this model
	std::vector<Mesh> meshes;
	// Texture names applied to meshes of this model. Has a 1:1 relation to meshes std::vector.
	std::vector<std::string> textures;

	void importModel(std::string filePath);
};