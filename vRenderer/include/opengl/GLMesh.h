#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"
#include "geometry_settings.h"

#include "GLUtils.h"

using namespace GLUtils;

class GLMesh
{
public:

	const uint32_t id;

	GLMesh(uint32_t id, const Mesh& mesh);
	~GLMesh();

	void draw();
	void setTransformMat(glm::mat4 transform);
private:

	uint32_t VBO, VAO;
	uint32_t EBO;

	uint32_t verticesCount;
	uint32_t indicesCount;

	// Transform
	glm::mat4 transformMat;

	void createFromGenericMesh(const Mesh& mesh);
	void cleanup();
};