#include "AssimpModelImporter.h"

std::shared_ptr<Model> AssimpModelImporter::importModel(std::filesystem::path modelFilePath, IImageAssetImporter& imageImporter, bool printImportData)
{
	// If model is already imported just return it
	if (importedModelsMap.find(modelFilePath.string()) != importedModelsMap.end())
	{
		return importedModelsMap[modelFilePath.string()];
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(modelFilePath.string(), ASSIMP_PREPROCESS_FLAGS);

	uint32_t meshCount = scene->mNumMeshes;
	uint32_t materialCount = 0;
	std::vector<std::unique_ptr<Mesh>> meshes(meshCount);
	std::vector<std::unique_ptr<Material>> materials(meshCount);

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

		meshes[i].reset(new Mesh(i, meshData->mName.C_Str(), vertices, indices, texCoords, normals));

		// Create a material if one is assigned to the mesh
		if (meshData->mMaterialIndex >= 0)
		{
			auto mat = scene->mMaterials[meshData->mMaterialIndex];
			std::string folderPath = modelFilePath.parent_path().string();

			// Texture loading lambda
			auto getTexture = [&mat, &folderPath, &imageImporter](aiTextureType type) {
				aiString path;
				std::shared_ptr<Texture> texture = nullptr;
				if (mat->GetTexture(type, 0, &path) == aiReturn_SUCCESS)
				{
					std::string s = path.C_Str();
					std::replace(s.begin(), s.end(), '/', '\\');
					s = folderPath + "\\" + s;
					texture = imageImporter.importTexture(s, false);
				}
				return texture;
				};
			
			Material* material = new Material(mat->GetName().C_Str());

			material->diffuseTexture = getTexture(aiTextureType_DIFFUSE);
			material->specularTexture = getTexture(aiTextureType_SPECULAR);
			material->ambientTexture = getTexture(aiTextureType_AMBIENT);
			material->emissionMap = getTexture(aiTextureType_EMISSIVE);
			material->normalMap = getTexture(aiTextureType_NORMALS);
			material->opacityMap = getTexture(aiTextureType_OPACITY);

			mat->Get(AI_MATKEY_SHININESS, material->shininess);
			mat->Get(AI_MATKEY_REFRACTI, material->refraction);
			if (material->opacityMap == nullptr)
				mat->Get(AI_MATKEY_OPACITY, material->opacity);

			auto getColor = [&](const char* key, uint32_t type, uint32_t idx, glm::vec3& color) {
				aiColor3D aiColor;
				mat->Get(key, type, idx, aiColor);
				color = glm::vec3(aiColor.r, aiColor.g, aiColor.b);
				};

			if (material->ambientTexture == nullptr)
				getColor(AI_MATKEY_COLOR_AMBIENT, material->ambientColor);
			if (material->diffuseTexture == nullptr)
				getColor(AI_MATKEY_COLOR_DIFFUSE, material->diffuseColor);
			if (material->specularTexture == nullptr)
				getColor(AI_MATKEY_COLOR_SPECULAR, material->specularColor);
			if (material->emissionMap == nullptr)
				getColor(AI_MATKEY_COLOR_EMISSIVE, material->emmissiveColor);

			materials[i].reset(material);
			materialCount++;
		}
	}

	if (printImportData)
	{
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

	// Assuming GLMaterial has a member 'opacity' (float or similar)
	auto sortByOpacity = [](std::vector<std::unique_ptr<Mesh>>& meshes, std::vector<std::unique_ptr<Material>>& materials) {
		// Check if vectors are the same size (1:1 relation)
		if (meshes.size() != materials.size()) {
			throw std::runtime_error("Vectors must have the same size for sorting");
		}

		// Create a vector of indices to sort
		std::vector<size_t> indices(meshes.size());
		std::iota(indices.begin(), indices.end(), 0); // Fill with 0, 1, 2, ..., N-1

		// Sort indices based on material opacity (ascending)
		std::sort(indices.begin(), indices.end(),
			[&materials](size_t a, size_t b) {
				return materials[a]->opacity > materials[b]->opacity; // Higher opacity comes first
			}
		);

		// Apply the sorted indices to both vectors
		std::vector< std::unique_ptr<Mesh>> sortedMeshes(meshes.size());
		std::vector< std::unique_ptr<Material>> sortedMaterials(materials.size());
		sortedMeshes.reserve(meshes.size());
		sortedMaterials.reserve(materials.size());

		// Move elements to their sorted positions
		for (size_t i = 0; i < indices.size(); i++) {
			sortedMeshes[i] = std::move(meshes[indices[i]]);
			sortedMaterials[i] = std::move(materials[indices[i]]);
		}
		// Move sorted vectors back to original vectors
		meshes = std::move(sortedMeshes);
		materials = std::move(sortedMaterials);
		};

	// Usage:
	sortByOpacity(meshes, materials);

	uint32_t id = importedModelsMap.size();
	std::shared_ptr<Model> newModel = std::make_shared<Model>(id, modelFilePath.stem().string(), std::move(meshes), std::move(materials), materialCount);
	importedModelsMap[modelFilePath.string()] = newModel;

	return newModel;
}