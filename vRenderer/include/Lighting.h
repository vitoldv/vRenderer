#pragma once

#include <glm/glm.hpp>
#include <string>

struct Light
{
	enum Type
	{
		// indexing starts from 1 because in shader 0 is treated as uninitialized (unprovided) light source
		DIRECTIONAL = 0,
		POINT = 1,
		SPOT = 2
	};

	const int id;
	Type type;

	// Attenuation terms
	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;

	glm::vec3 color = { 1.0f, 1.0f, 1.0f };

	glm::vec3 position = {0, 0, 0};
	glm::vec3 direction = {0, -1.0f, 0};
	float cutOff = 20.0f;
	float outerCutOff = 25.0f;

	Light(int id, Type type) : id(id), type(type) {}
};
