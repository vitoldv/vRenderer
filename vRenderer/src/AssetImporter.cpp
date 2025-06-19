#include "AssetImporter.h"

AssetImporter::AssetImporter(IModelAssetImporter* modelImporter, IImageAssetImporter* imageImporter)
{
	this->modelImporter.reset(modelImporter);
	this->imageImporter.reset(imageImporter);
}

std::shared_ptr<Model> AssetImporter::importModel(std::string modelName)
{
	std::this_thread::sleep_for(std::chrono::seconds(5));
	std::string modelFileName = modelName + "\\" + modelName + ".obj";
	std::filesystem::path path = MODEL_ASSETS(modelFileName.c_str());
	return modelImporter->importModel(path, *imageImporter, true);
}

Model_future AssetImporter::importModel_async(std::string modelName)
{
	return std::async(std::launch::async, [this, modelName]() {
		return importModel(modelName);
		});
	//std::thread([this, modelName, onFinish]() {
	//	auto model = importModel(modelName);
	//	MainThreadDispatcher::postWithArgs(onFinish, model);
	//	}).detach();
}