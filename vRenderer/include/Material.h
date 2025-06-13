#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <glm/glm.hpp>

#include <Texture.h>
/*
	Generic imported material
*/

class Material
{
public:

	const std::string name;

	float shininess;
	float opacity = 0;
	glm::vec3 ambientColor = {};
	glm::vec3 diffuseColor = {};
	glm::vec3 specularColor = {};

	glm::vec3 emmissiveColor = {};			// not used
	float refraction;						// not used

	std::shared_ptr<Texture> ambientTexture = {};
	std::shared_ptr<Texture> diffuseTexture = {};
	std::shared_ptr<Texture> specularTexture = {};

	std::shared_ptr<Texture> opacityMap = {};
	std::shared_ptr<Texture> emissionMap = {};

	std::shared_ptr<Texture> normalMap = {};			// not used

	Material(const char* name);
};