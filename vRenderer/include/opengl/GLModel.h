#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLMesh.h"
#include "GLShader.h"
#include "GLMaterial.h"
#include "GLUtils.h"
#include "Model.h"
#include "BaseCamera.h"

class GLModel
{
public:
	const uint32_t id;

	GLModel(uint32_t id, const Model& model);
	~GLModel();

	void draw(GLShader& shader, BaseCamera& camera);
	void setTransform(glm::mat4 transform);
	const glm::mat4 getTransform() const;

private:
	
	const char* MODEL_UNIFORM_NAME = "model";
	const char* NORMAL_MATRIX_UNIFORM_NAME = "normalMatrix";
	const char* USE_MATERIAL_UNIFORM_NAME = "useMaterial";

	uint32_t meshCount;
	uint32_t materialCount;

	glm::mat4 transform;

	std::vector<GLMesh*> meshes;
	std::vector<GLMaterial*> materials;

	void createFromGenericModel(const Model& model);

	void cleanup();
};