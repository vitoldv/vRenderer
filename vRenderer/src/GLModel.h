#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "GLMesh.h"
#include "GLTexture.h"
#include "GLUtils.h"
#include "Model.h"

class GLModel
{
public:
	const uint32_t id;

	GLModel(uint32_t id, const Model& model);
	~GLModel();

	void draw(uint32_t shaderProgram);
	void setTransform(glm::mat4 transform);

private:
	
	const char* TRANSFORM_UNIFORM_NAME = "transform";
	const char* SAMPLER_UNIFORM_NAME = "outTexture";
	const char* USE_TEXTURE_UNIFORM_NAME = "useTexture";

	uint32_t meshCount;
	uint32_t textureCount;

	glm::mat4 transform;

	std::vector<GLMesh*> meshes;
	std::vector<GLTexture*> textures;

	void createFromGenericModel(const Model& model);

	void cleanup();
};