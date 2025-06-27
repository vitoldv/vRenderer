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