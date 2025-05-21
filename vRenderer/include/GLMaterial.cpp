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

void GLMaterial::apply(GLShader& shader)
{
	shader.setUniform(DIFFUSE_COLOR_UNIFORM_NAME, diffuseColor);

	if (diffuse != nullptr)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuse->glId);
		shader.setUniform(DIFFUSE_SAMPLER_UNIFORM_NAME, 0);
	}
	
	shader.setUniform(USE_DIFFUSE_COLOR_UNIFORM_NAME, diffuse != nullptr ? GL_FALSE : GL_TRUE);

	if (specular != nullptr)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specular->glId);
		shader.setUniform(SPECULAR_SAMPLER_UNIFORM_NAME, 1);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLMaterial::cleanup()
{
	delete diffuse;
	diffuse = nullptr;

	delete specular;
	specular = nullptr;
}
