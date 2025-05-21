#include "Model.h"
#include <iostream>

Model::Model(uint32_t id, std::string filePath) :
	id(id),
	folderPath(filePath.substr(0, filePath.find_last_of('\\')))
{
	importModel(filePath);
}

uint32_t Model::getMeshCount() const
{
	return this->meshCount;
}

uint32_t Model::getMaterialCount() const
{
	return this->materialsCount;
}

std::string Model::getName()
{
	return this->name;
}

const std::vector<Mesh>& Model::getMeshes() const
{
	return this->meshes;
}

const std::vector<Material*>& Model::getMaterials() const
{
	return this->materials;
}

void Model::importModel(std::string filePath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, ASSIMP_PREPROCESS_FLAGS);
	
	// Take a name from scene, if it's empty - extract it from model file path
	if (this->name.empty())
	{
		int start = filePath.find_last_of('\\') + 1;
		int end = filePath.find_last_of('.');
		this->name = filePath.substr(start, end - start);
	}

	this->meshCount = scene->mNumMeshes;

	this->materials.resize(meshCount);

	// Import meshes and textures
	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		auto meshData = scene->mMeshes[i];
		std::vector<glm::vec3> vertices(meshData->mNumVertices);
		std::vector<glm::vec3> normals(meshData->mNumVertices);
		std::vector<glm::vec2> texCoords(meshData->mNumVertices);
		std::vector<uint32_t> indices(meshData->mNumFaces * 3);
		for (int j = 0; j < meshData->mNumVertices; j++)
		{
			vertices[j] = glm::vec3(meshData->mVertices[j].x, meshData->mVertices[j].y, meshData->mVertices[j].z);
			normals[j] = glm::vec3(meshData->mNormals[j].x, meshData->mNormals[j].y, meshData->mNormals[j].z);
			texCoords[j] = glm::vec2(meshData->mTextureCoords[0][j].x, meshData->mTextureCoords[0][j].y);
		}
		for (int j = 0; j < meshData->mNumFaces; j++)
		{
			auto face = meshData->mFaces[j];
			indices[j * 3] = face.mIndices[0] + VERTEX_INDEX_OFFSET;
			indices[j * 3 + 1] = face.mIndices[1] + VERTEX_INDEX_OFFSET;
			indices[j * 3 + 2] = face.mIndices[2] + VERTEX_INDEX_OFFSET;
		}

		this->meshes.push_back(Mesh(i, meshData->mName.C_Str(), vertices, indices, texCoords, normals));

		// Create a material if one is assigned to the mesh
		if (meshData->mMaterialIndex >= 0)
		{
			auto mat = scene->mMaterials[meshData->mMaterialIndex];
			std::string folderPath = this->folderPath;
			auto getTexture = [mat, folderPath](aiTextureType type) {
				aiString path;
				if (mat->GetTexture(type, 0, &path) == aiReturn_SUCCESS)
				{
					std::string s = path.C_Str();
					std::replace(s.begin(), s.end(), '/', '\\');
					s = folderPath + "\\" + s;
					return s;
				}
				return std::string("");
			};

			Material* material = new Material(mat->GetName().C_Str());
			material->diffuseTexture = getTexture(aiTextureType_DIFFUSE);
			material->specularTexture = getTexture(aiTextureType_SPECULAR);

			this->materials[i] = material;
			this->materialsCount++;
		}		
	}

	for (int i = 0; i < scene->mNumMaterials; i++)
	{
		auto mat = scene->mMaterials[i];
		for (int j = 0; j <= 25; j++)
		{
			aiString path;
			if (mat->GetTexture(static_cast<aiTextureType>(j), 0, &path) == aiReturn_SUCCESS)
			{
				std::cout << aiTextureTypeToString(static_cast<aiTextureType>(j)) << " " << path.C_Str() << " found in mat " << mat->GetName().C_Str() << std::endl;
			}
		}
	}
}
