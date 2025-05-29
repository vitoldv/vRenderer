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
	shader.setUniform(DIFFUSE_COLOR_UNIFORM_NAME, diffuseColor);
	shader.setUniform(USE_DIFFUSE_COLOR_UNIFORM_NAME, diffuse != nullptr ? GL_FALSE : GL_TRUE);

	auto applyTexture = [&shader](const char* uniform, GLTexture* texture, int index) {
		if (texture == nullptr) return;
		glActiveTexture(GL_TEXTURE0 + index);
		glBindTexture(GL_TEXTURE_2D, texture->glId);
		shader.setUniform(uniform, index);
	};

	applyTexture(DIFFUSE_SAMPLER_UNIFORM_NAME, diffuse, 0);
	applyTexture(SPECULAR_SAMPLER_UNIFORM_NAME, specular, 1);
}

void GLMaterial::cleanup()
{
	delete diffuse;
	diffuse = nullptr;

	delete specular;
	specular = nullptr;
}
