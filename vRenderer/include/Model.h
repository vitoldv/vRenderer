#pragma once

#include <vector>
#include <string>

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

	Model(uint32_t id, std::string filePath);

	std::vector<Mesh>* getMeshes();
	std::vector<std::string>* getTextures();

private:
	// Meshes of this model
	std::vector<Mesh> meshes;
	// Texture names applied to meshes of this model. Has a 1:1 relation to meshes vector.
	std::vector<std::string> textures;

	void importModel(std::string filePath);
};