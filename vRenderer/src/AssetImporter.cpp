#include "AssetImporter.h"

AssetImporter::AssetImporter(IModelAssetImporter* modelImporter, IImageAssetImporter* imageImporter)
{
	this->modelImporter.reset(modelImporter);
	this->imageImporter.reset(imageImporter);
	this->pool = std::make_unique<ThreadPool>(2);
}

std::shared_ptr<Model> AssetImporter::importModel(std::string modelName)
{
	std::filesystem::path modelFolderPath;
	for (const auto& modelFolder : std::filesystem::directory_iterator(MODEL_ASSETS_FOLDER))
	{
		if (modelFolder.path().stem() == modelName)
		{
			modelFolderPath = modelFolder;
			break;
		}
	}
	
	std::filesystem::path modelFile;
	for (const auto& file : std::filesystem::directory_iterator(modelFolderPath))
	{
		if (file.path().stem() == modelName)
		{
			modelFile = file.path();
		}
	}

	return modelImporter->importModel(modelFile, *imageImporter, true);
}

std::shared_ptr<Texture> AssetImporter::importTexture(std::string textureName)
{
	return imageImporter->importTexture(textureName, true);
}

/// <summary>
/// Imports a cubemap (6 distinct textures).
/// The provided name is expected to be the name of a folder, that contains 6 images of an
/// appropriate extensions with appropriate names for each cubemap's face,
/// i.e. "back", "bottom", "front", "left", "right", "top"
/// </summary>
/// <param name="cubemapName"></param>
/// <returns></returns>
std::shared_ptr<Cubemap> AssetImporter::importCubemap(std::string cubemapName)
{
	std::filesystem::path path = CUBEMAP_ASSETS(cubemapName.c_str());
	return imageImporter->importCubemap(path, true);
}
