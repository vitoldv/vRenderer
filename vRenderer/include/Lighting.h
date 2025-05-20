#pragma once

#include <glm/glm.hpp>

struct LightSource
{
	float ambientStrength;
	float specularStrength;
	int shininess;

	glm::vec3 color;
	glm::vec3 position;
};