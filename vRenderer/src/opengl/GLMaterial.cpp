#include "GLMaterial.h"

GLMaterial::GLMaterial(const char* name) :
	name(name)
{
	diffuse = nullptr;
	specular = nullptr;
}

GLMaterial::~GLMaterial()
{
	cleanup();
}

/// <summary>
///  Bind material resources to shader
/// </summary>
/// <param name="shader"></param>
void GLMaterial::apply(GLShader& shader)
{
	shader.setUniform(AMBIENT_COLOR_UNIFORM_NAME, ambientColor);
	shader.setUniform(DIFFUSE_COLOR_UNIFORM_NAME, diffuseColor);
	shader.setUniform(SPECULAR_COLOR_UNIFORM_NAME, specularColor);
	shader.setUniform(OPACITY_UNIFORM_NAME, opacity);
	shader.setUniform(SHININESS_UNIFORM_NAME, shininess);

	auto applyTexture = [&shader](const char* uniform, GLTexture* texture, int index) {
		glActiveTexture(GL_TEXTURE0 + index);
		glBindTexture(GL_TEXTURE_2D, 0);
		if (texture == nullptr) return;
		glActiveTexture(GL_TEXTURE0 + index);
		glBindTexture(GL_TEXTURE_2D, texture->glId);
		shader.setUniform(uniform, index);
	};

	applyTexture(AMBIENT_SAMPLER_UNIFORM_NAME, ambient, 0);
	applyTexture(DIFFUSE_SAMPLER_UNIFORM_NAME, diffuse, 1);
	applyTexture(SPECULAR_SAMPLER_UNIFORM_NAME, specular, 2);
	applyTexture(OPACITY_SAMPLER_UNIFORM_NAME, opacityMap, 3);
}

void GLMaterial::cleanup()
{
	delete diffuse;
	diffuse = nullptr;

	delete specular;
	specular = nullptr;
}
