#include "AssetImporter.h"

AssetImporter::AssetImporter(IModelAssetImporter* modelImporter, IImageAssetImporter* imageImporter)
{
	this->modelImporter.reset(modelImporter);
	this->imageImporter.reset(imageImporter);
}

std::shared_ptr<Model> AssetImporter::importModel(std::string modelName)
{
	std::string modelFileName = modelName + "\\" + modelName + ".obj";
	std::filesystem::path path = MODEL_ASSETS(modelFileName.c_str());
	return modelImporter->importModel(path, *imageImporter, true);
}