#include "GLModel.h"

GLModel::GLModel(uint32_t id, const Model& model) : id(id)
{
	createFromGenericModel(model);
}

GLModel::~GLModel()
{
	cleanup();
}

void GLModel::setTransform(glm::mat4 transform)
{
	this->transform = transform;
	for (auto* mesh : meshes)
	{
		mesh->setTransformMat(transform);
	}
}

void GLModel::draw(GLShader& shader)
{
	// Calculate Normal matrix (required for proper normals transformation)
	glm::mat3 normalMat = glm::transpose(glm::inverse(this->transform));

	// Setting uniforms
	shader.setUniform(MODEL_UNIFORM_NAME, this->transform);
	shader.setUniform(NORMAL_MATRIX_UNIFORM_NAME, normalMat);

	for (int i = 0; i < meshes.size(); i++)
	{
		if (textures[i] != nullptr)
		{
			// Attach texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[i]->glId);
			shader.setUniform(SAMPLER_UNIFORM_NAME, 0);
		}
		else
		{
			shader.setUniform(AMBIENT_COLOR_UNIFORM_NAME, ambientColor);
		}
		
		shader.setUniform(USE_TEXTURE_UNIFORM_NAME, textures[i] != nullptr ? GL_TRUE : GL_FALSE);

		meshes[i]->draw();
	}
}

void GLModel::createFromGenericModel(const Model& model)
{
	meshCount = model.getMeshCount();
	meshes.resize(meshCount);
	textures.resize(meshCount);

	for (int i = 0; i < meshCount; i++)
	{
		const Mesh& mesh = model.getMeshes()[i];

		uint32_t newMeshId = i;
		GLMesh* glMesh = new GLMesh(newMeshId, mesh);

		auto textureName = model.getTextures()[i];
		GLTexture* glTexture = nullptr;
		if (!textureName.empty())
		{
			textureCount++;
			glTexture = new GLTexture(model.getFullTexturePath(i));
		}

		meshes[i] = glMesh;
		textures[i] = glTexture;
	}
}

void GLModel::cleanup()
{
	for (auto& texture : textures)
	{
		delete texture;
		texture = nullptr;
	}
	for (auto& mesh : meshes)
	{
		delete mesh;
		mesh = nullptr;
	}
}
