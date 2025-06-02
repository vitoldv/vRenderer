#pragma once

#include <string>

#include "GLShader.h"
#include "GLTexture.h"

class GLMaterial
{
public:
	
	const std::string name;

	GLTexture* ambient;
	GLTexture* diffuse;
	GLTexture* specular;
	GLTexture* opacityMap;

	float shininess;
	float opacity;
	glm::vec3 ambientColor;
	glm::vec3 diffuseColor;
	glm::vec3 specularColor;

	GLMaterial(const char* name);
	~GLMaterial();

	void apply(GLShader& shader);
	void cleanup();

private:

	const char* AMBIENT_SAMPLER_UNIFORM_NAME = "material.ambientMap";
	const char* DIFFUSE_SAMPLER_UNIFORM_NAME = "material.diffuseMap";
	const char* SPECULAR_SAMPLER_UNIFORM_NAME = "material.specularMap";
	const char* OPACITY_SAMPLER_UNIFORM_NAME = "material.opacityMap";

	const char* AMBIENT_COLOR_UNIFORM_NAME = "material.ambientColor";
	const char* DIFFUSE_COLOR_UNIFORM_NAME = "material.diffuseColor";
	const char* SPECULAR_COLOR_UNIFORM_NAME = "material.specularColor";

	const char* OPACITY_UNIFORM_NAME = "material.opacity";
	const char* SHININESS_UNIFORM_NAME = "material.shininess";
};
