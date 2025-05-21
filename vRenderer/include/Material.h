#pragma once

#include <string>

/*
	Generic imported material
*/
class Material
{
public:

	const char* name;
	std::string diffuseTexture = {};
	std::string specularTexture = {};

	Material(const char* name);
};