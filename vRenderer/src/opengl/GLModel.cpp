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

const glm::mat4 GLModel::getTransform() const
{
	return transform;
}

void GLModel::draw(GLShader& shader, BaseCamera& camera)
{
	// Calculate Normal matrix (required for proper normals transformation)
	glm::mat3 normalMat = glm::transpose(glm::inverse(camera.getViewMatrix() * this->transform));

	// Setting uniforms
	shader.setUniform(MODEL_UNIFORM_NAME, this->transform);
	shader.setUniform(NORMAL_MATRIX_UNIFORM_NAME, normalMat);
	
	for (int i = 0; i < meshes.size(); i++)
	{
		if ((1 << i & meshMask) == 0)
		{
			continue;
		}
		if (materials[i] != nullptr)
		{
			materials[i]->apply(shader);
		}
			
		meshes[i]->draw();
	}
}

void GLModel::createFromGenericModel(const Model& model)
{
	meshCount = model.getMeshCount();
	meshes.resize(meshCount);
	materials.resize(meshCount);

	for (int i = 0; i < meshCount; i++)
	{
		const Mesh& mesh = model.getMeshes()[i];

		uint32_t newMeshId = i;
		GLMesh* glMesh = new GLMesh(newMeshId, mesh);

		auto material = model.getMaterials()[i];
		if (material != nullptr)
		{
			materialCount++;
			GLMaterial* glMaterial = new GLMaterial(*material);
			materials[i] = glMaterial;
		}
		meshes[i] = glMesh;
	}
}

void GLModel::cleanup()
{
	for (auto& material : materials)
	{
		delete material;
		material = nullptr;
	}
	for (auto& mesh : meshes)
	{
		delete mesh;
		mesh = nullptr;
	}
}
