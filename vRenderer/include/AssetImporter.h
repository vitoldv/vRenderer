#pragma once

#include <filesystem>
#include <thread>
#include <future>

#include "IModelAssetImporter.h"
#include "IImageAssetImporter.h"
#include "ThreadDispatcher.h"

#define ASSETS_FOLDER "vRenderer\\assets\\"
#define MODEL_ASSETS_FOLDER "vRenderer\\assets\\models\\"
#define MODEL_ASSETS(asset) concat(MODEL_ASSETS_FOLDER, asset)

class AssetImporter
{
public:

	AssetImporter(IModelAssetImporter* modelImporter, IImageAssetImporter* imageImporter);

	std::shared_ptr<Model> importModel(std::string modelName);

	template<typename Callback>
	void importModel_async(std::string modelName, Callback onFinish);

private:

	std::unique_ptr<IModelAssetImporter> modelImporter;
	std::unique_ptr<IImageAssetImporter> imageImporter;

	std::future<void> future;
};


// Helper function for runtime concatenation
static inline const char* concat(const char* a, const char* b) {
	static char buffer[256]; // Ensure buffer is large enough
	snprintf(buffer, sizeof(buffer), "%s%s", a, b);
	return buffer;
}

template<typename Callback>
void AssetImporter::importModel_async(std::string modelName, Callback onFinish)
{
	future = std::async(std::launch::async, [this, modelName, onFinish]() {
		auto model = importModel(modelName);
		MainThreadDispatcher::postWithArgs(onFinish, model);
	});
}