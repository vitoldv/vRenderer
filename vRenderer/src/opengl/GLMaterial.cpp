#include "GLMaterial.h"

GLMaterial::GLMaterial(const Material& material) :
	name(material.name)
{
	createFromGenericMaterial(material);
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

	auto applyTexture = [&](const char* uniform, GLTexture* texture, int index) {
		// Clear binding
		glActiveTexture(GL_TEXTURE0 + index);
		glBindTexture(GL_TEXTURE_2D, 0);
		// Bind if texture exists
		if (texture == nullptr) return;
		glActiveTexture(GL_TEXTURE0 + index);
		glBindTexture(GL_TEXTURE_2D, texture->glId);
		shader.setUniform(uniform, index);
	};

	applyTexture(AMBIENT_SAMPLER_UNIFORM_NAME, ambient.get(), 0);
	applyTexture(DIFFUSE_SAMPLER_UNIFORM_NAME, diffuse.get(), 1);
	applyTexture(SPECULAR_SAMPLER_UNIFORM_NAME, specular.get(), 2);
	applyTexture(OPACITY_SAMPLER_UNIFORM_NAME, opacityMap.get(), 3);
}

void GLMaterial::createFromGenericMaterial(const Material& material)
{
	if (material.ambientTexture != nullptr)
	{
		ambient = std::make_unique<GLTexture>(*material.ambientTexture);
	}
	if (material.diffuseTexture != nullptr)
	{
		diffuse = std::make_unique<GLTexture>(*material.diffuseTexture);
	}
	if (material.specularTexture != nullptr)
	{
		specular = std::make_unique<GLTexture>(*material.specularTexture);
	}
	if (material.opacityMap != nullptr)
	{
		opacityMap = std::make_unique<GLTexture>(*material.opacityMap);
	}

	ambientColor = material.ambientColor;
	diffuseColor = material.diffuseColor;
	specularColor = material.specularColor;

	opacity = material.opacity;
	shininess = material.shininess;
}
