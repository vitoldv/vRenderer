#include "Mesh.h"

Mesh::Mesh(int id, const char* name, std::vector<glm::vec3> vertices, std::vector<uint32_t> indices,
    std::vector<glm::vec2> texCoords, std::vector<glm::vec3> normals) : id(id), name(name)
{
    this->vertices = vertices;
    this->indices = indices;
    this->texCoords = texCoords;
    this->normals = normals;
}

const std::vector<glm::vec3>& Mesh::getVertices() const
{
    return this->vertices;
}

const std::vector<glm::vec2>& Mesh::getTexCoords() const
{
    return this->texCoords;
}

const std::vector<glm::vec3>& Mesh::getNormals() const
{
    return this->normals;
}

const std::vector<uint32_t>& Mesh::getIndices() const
{
    return this->indices;
}