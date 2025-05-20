#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLMesh.h"
#include "GLShader.h"
#include "GLTexture.h"
#include "GLUtils.h"
#include "Model.h"

class GLModel
{
public:
	const uint32_t id;

	GLModel(uint32_t id, const Model& model);
	~GLModel();

	void draw(GLShader& shader);
	void setTransform(glm::mat4 transform);

private:
	
	const char* MODEL_UNIFORM_NAME = "model";
	const char* NORMAL_MATRIX_UNIFORM_NAME = "normalMatrix";
	const char* SAMPLER_UNIFORM_NAME = "outTexture";
	const char* USE_TEXTURE_UNIFORM_NAME = "useTexture";
	const char* AMBIENT_COLOR_UNIFORM_NAME = "ambientColor";

	const glm::vec3 ambientColor = { 1.0f, 0.5f, 0.31f };
	const glm::vec3 diffuseColor = { 1.0f, 0.5f, 0.31f };

	uint32_t meshCount;
	uint32_t textureCount;

	glm::mat4 transform;

	std::vector<GLMesh*> meshes;
	std::vector<GLTexture*> textures;

	void createFromGenericModel(const Model& model);

	void cleanup();
};