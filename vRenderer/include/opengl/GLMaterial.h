#pragma once

#include <string>

#include "GLShader.h"
#include "GLTexture.h"
#include "Material.h"

class GLMaterial
{
public:
	
	const std::string name;

	std::unique_ptr<GLTexture> ambient		= nullptr;
	std::unique_ptr<GLTexture> diffuse		= nullptr;
	std::unique_ptr<GLTexture> specular		= nullptr;
	std::unique_ptr<GLTexture> opacityMap	= nullptr;

	float shininess							= 1.0f;
	float opacity							= 1.0f;
	glm::vec3 ambientColor					= {};
	glm::vec3 diffuseColor					= {};
	glm::vec3 specularColor					= {};

	GLMaterial(const Material& material);

	void apply(GLShader& shader);

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

	void createFromGenericMaterial(const Material& material);
};
