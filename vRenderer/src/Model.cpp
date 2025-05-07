#include "Model.h"

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

std::string Model::getName()
{
	return this->name;
}

const std::vector<Mesh>& Model::getMeshes() const
{
	return this->meshes;
}

const std::vector<std::string>& Model::getTextures() const
{
	return this->textures;
}

/// <summary>
/// Returns full path to texture. 
/// If texture at this index is present, returns this texture.
/// If there is no texture at this index or index is invalid, returns empty string.
/// </summary>
/// <param name="textureIndex"></param>
/// <returns></returns>
std::string Model::getFullTexturePath(int textureIndex) const
{
	std::string path = "";
	if (textureIndex < this->meshCount && textureIndex >= 0)
	{
		if (!this->textures[textureIndex].empty())
		{
			path = this->folderPath + "\\" + this->textures[textureIndex];
		}
	}
	return path;
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

		// If mesh has a material assigned and this material has a diffuse texture
		// we save a path to the texture std::vector at the same index as mesh in corresponding mesh std::vector
		if (meshData->mMaterialIndex >= 0)
		{
			auto mat = scene->mMaterials[meshData->mMaterialIndex];

			aiString path;
			if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == aiReturn_SUCCESS)
			{
				this->textures.push_back(std::string(path.C_Str()));
				std::replace(this->textures[i].begin(), this->textures[i].end(), '/', '\\');
			}
		}		
	}
}
