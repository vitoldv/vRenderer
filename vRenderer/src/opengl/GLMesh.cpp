#include "GLMesh.h"

GLMesh::GLMesh(uint32_t id, const Mesh& mesh) : id(id)
{
	createFromGenericMesh(mesh);
}

GLMesh::~GLMesh()
{
	cleanup();
}

void GLMesh::createFromGenericMesh(const Mesh& mesh)
{
	const auto& meshVertices = mesh.getVertices();
	const auto& meshIndices = mesh.getIndices();
	const auto& meshTexCoords = mesh.getTexCoords();
	const auto& meshNormals = mesh.getNormals();

	verticesCount = meshVertices.size();
	indicesCount = meshIndices.size();
	std::vector<Vertex> vertices(verticesCount);
	for (int i = 0; i < verticesCount; i++)
	{
		Vertex vertex = {};
		vertex.pos = meshVertices.at(i);
		vertex.color = glm::vec3(1.0f);
		vertex.normal = meshNormals.at(i);
		vertex.uv = meshTexCoords.at(i);
		vertices[i] = vertex;
	}

	// Create vertex data (VBO and VAO) and index data (EBO)
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);  // <-- Add this before VBO setup

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshIndices.size() * sizeof(uint32_t), meshIndices.data(), GL_STATIC_DRAW);
	
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
	glEnableVertexAttribArray(1);
	// normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	// texture coords attribute
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glEnableVertexAttribArray(3);	
}

void GLMesh::draw()
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glDrawElementsBaseVertex(GL_TRIANGLES, this->indicesCount, GL_UNSIGNED_INT, (void*)0, -VERTEX_INDEX_OFFSET);
	glBindVertexArray(0);
}

void GLMesh::setTransformMat(glm::mat4 transform)
{
	this->transformMat = transform;
}

void GLMesh::cleanup()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}