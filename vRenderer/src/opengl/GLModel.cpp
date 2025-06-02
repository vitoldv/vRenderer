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


	// Assuming GLMaterial has a member 'opacity' (float or similar)
	auto sortByOpacity = [](std::vector<GLMesh*>& meshes, std::vector<GLMaterial*>& materials) {
		// Check if vectors are the same size (1:1 relation)
		if (meshes.size() != materials.size()) {
			throw std::runtime_error("Vectors must have the same size for sorting");
		}

		// Create a vector of indices to sort
		std::vector<size_t> indices(meshes.size());
		std::iota(indices.begin(), indices.end(), 0); // Fill with 0, 1, 2, ..., N-1

		// Sort indices based on material opacity (ascending)
		std::sort(indices.begin(), indices.end(),
			[&materials](size_t a, size_t b) {
				return materials[a]->opacity > materials[b]->opacity; // Higher opacity comes first
			}
		);

		// Apply the sorted indices to both vectors
		std::vector<GLMesh*> sortedMeshes;
		std::vector<GLMaterial*> sortedMaterials;
		sortedMeshes.reserve(meshes.size());
		sortedMaterials.reserve(materials.size());

		for (size_t i : indices) {
			sortedMeshes.push_back(meshes[i]);
			sortedMaterials.push_back(materials[i]);
		}

		// Swap the original vectors with the sorted ones
		meshes.swap(sortedMeshes);
		materials.swap(sortedMaterials);
	};

	// Usage:
	sortByOpacity(meshes, materials);
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
