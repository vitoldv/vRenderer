#pragma once

#include <string>

#include "GLShader.h"
#include "GLTexture.h"

class GLMaterial
{
public:
	
	const std::string name;

	GLTexture* diffuse;
	GLTexture* specular;

	GLMaterial(const char* name);
	~GLMaterial();

	void apply(GLShader& shader);
	void cleanup();

private:

	const char* DIFFUSE_SAMPLER_UNIFORM_NAME = "material.diffuseMap";
	const char* SPECULAR_SAMPLER_UNIFORM_NAME = "material.specularMap";
	const char* DIFFUSE_COLOR_UNIFORM_NAME = "material.diffuseColor";

	const char* USE_DIFFUSE_COLOR_UNIFORM_NAME = "useDiffuseColor";

	// Color applied to fragment if diffuse texture is missing
	const glm::vec3 diffuseColor = { 1.0f, 0.5f, 0.31f };
};
