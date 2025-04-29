#include "Model.h"

Model::Model(uint32_t id, std::string filePath)
{
	this->id = id;
	importModel(filePath);
}

std::vector<Mesh>* Model::getMeshes()
{
	return &this->meshes;
}

std::vector<std::string>* Model::getTextures()
{
	return &this->textures;
}

void Model::importModel(std::string filePath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, ASSIMP_PREPROCESS_FLAGS);
	
	// Take a name from scene, if it's empty - extract it from model file path
	this->name = scene->mName.C_Str();
	if (this->name.empty())
	{
		int start = filePath.find_last_of('\\') + 1;
		int end = filePath.find_last_of('.');
		this->name = filePath.substr(start, end - start);
	}

	this->meshes.resize(scene->mNumMeshes);
	this->textures.resize(scene->mNumMeshes);

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

		this->meshes[i] = Mesh(i, meshData->mName.C_Str(), vertices, indices, texCoords, normals);

		// If mesh has a material assigned and this material has a diffuse texture
		// we save a path to the texture vector at the same index as mesh in corresponding mesh vector
		if (meshData->mMaterialIndex >= 0)
		{
			auto mat = scene->mMaterials[meshData->mMaterialIndex];

			aiString path;
			if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == aiReturn_SUCCESS)
			{
				this->textures[i] = path.C_Str();
			}
		}		
	}
}
