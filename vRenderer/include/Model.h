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
	uint32_t id;
	std::string name;	
	std::string folderPath;

	Model(uint32_t id, std::string filePath);

	int getMeshesCount();
	std::vector<Mesh>* getMeshes();
	std::vector<std::string>* getTextures();
	std::string getFullTexturePath(int textureIndex);

private:
	// Meshes of this model
	int meshesCount;
	std::vector<Mesh> meshes;
	// Texture names applied to meshes of this model. Has a 1:1 relation to meshes std::vector.
	std::vector<std::string> textures;

	void importModel(std::string filePath);
};