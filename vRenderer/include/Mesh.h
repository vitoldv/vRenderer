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
	int id;
	std::string name;

	Mesh();
	Mesh(int id, const char* name, std::vector<glm::vec3> vertices, std::vector<uint32_t> indices,
		std::vector<glm::vec2> texCoords, std::vector<glm::vec3> normals);
	~Mesh();

	std::vector<glm::vec3>& getVertices();
	std::vector<glm::vec2>& getTexCoords();
	std::vector<glm::vec3>& getNormals();
	std::vector<uint32_t>& getIndices();

private:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<uint32_t> indices;
};