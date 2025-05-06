#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

/*
	Generic class of a mesh from imported model
*/

class Mesh
{
public:
	const int id;
	const std::string name;

	Mesh(int id, const char* name, std::vector<glm::vec3> vertices, std::vector<uint32_t> indices,
		std::vector<glm::vec2> texCoords, std::vector<glm::vec3> normals);
	~Mesh() = default;

	const std::vector<glm::vec3>& getVertices() const;
	const std::vector<glm::vec2>& getTexCoords() const;
	const std::vector<glm::vec3>& getNormals() const;
	const std::vector<uint32_t>& getIndices() const;

private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<uint32_t> indices;
};