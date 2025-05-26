#pragma once

#include <glm/glm.hpp>
#include <string>

#include <GLShader.h>

struct Light
{
	enum Type
	{
		// indexing starts from 1 because in shader 0 is treated as uninitialized (unprovided) light source
		DIRECTIONAL = 1,
		POINT = 2,
		SPOT = 3
	};

	const int id;
	const Type type;

	// Phong shading parameters
	float ambientStrength;
	float specularStrength;
	int shininess;

	// Attenuation terms
	float constant;
	float linear;
	float quadratic;

	glm::vec3 color;

	glm::vec3 position;
	glm::vec3 direction;
	float cutOff;
	float outerCutOff;

	Light(int id, Type type) : id(id), type(type) {}
};
