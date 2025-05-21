#pragma once

#include <string>

/*
	Generic imported material
*/
class Material
{
public:

	const std::string name;
	std::string diffuseTexture = {};
	std::string specularTexture = {};

	Material(const char* name);
};