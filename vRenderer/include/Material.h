#pragma once

#include <string>
#include <glm/glm.hpp>

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

	std::string ambientTexture = {};
	std::string diffuseTexture = {};
	std::string specularTexture = {};

	std::string opacityMap = {};
	std::string emissionMap = {};

	std::string normalMap = {};			// not used

	Material(const char* name);
};