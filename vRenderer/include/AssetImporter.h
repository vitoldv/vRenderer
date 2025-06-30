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

using Model_future = std::future<std::shared_ptr<Model>>;

class AssetImporter
{
public:

	AssetImporter(IModelAssetImporter* modelImporter, IImageAssetImporter* imageImporter);

	std::shared_ptr<Model> importModel(std::string modelName);
	std::shared_ptr<Texture> importTexture(std::string textureName);

	template<typename Callback>
	void importModel_async(std::string modelName, Callback onFinish);

	template<typename Callback>
	void importTexture_async(std::string textureName, Callback onFinish);

	template<typename Callback>
	void importTextures_async(const std::vector<std::string>& textureNames, Callback onFinish);

private:

	std::unique_ptr<ThreadPool> pool;
	std::unique_ptr<IModelAssetImporter> modelImporter;
	std::unique_ptr<IImageAssetImporter> imageImporter;

};

// Helper function for runtime concatenation
static inline const char* concat(const char* a, const char* b) {
	static char buffer[256]; // Ensure buffer is large enough
	snprintf(buffer, sizeof(buffer), "%s%s", a, b);
	return buffer;
}

template<typename Callback>
inline void AssetImporter::importModel_async(std::string modelName, Callback onFinish)
{
	auto* dispatcher = &ThreadDispatcher::instance();
	dispatcher->worker([this, modelName, onFinish, dispatcher]() {
		auto model = importModel(modelName);
		dispatcher->main(onFinish, model);
		});
}

template<typename Callback>
inline void AssetImporter::importTexture_async(std::string textureName, Callback onFinish)
{
	auto* dispatcher = &ThreadDispatcher::instance();
	dispatcher->worker([this, textureName, onFinish, dispatcher]() {
		auto texture = importTexture(textureName);
		dispatcher->main(onFinish, texture);
		});
}

template<typename Callback>
inline void AssetImporter::importTextures_async(const std::vector<std::string>& textureNames, Callback onFinish)
{
	auto* dispatcher = &ThreadDispatcher::instance();
	dispatcher->worker([this, textureNames, onFinish, dispatcher]() {
		std::vector<std::shared_ptr<Texture>> importedTextures(textureNames.size());
		for (int i = 0; i < textureNames.size(); ++i)
		{
			importedTextures[i] = importTexture(textureNames[i]);
		}
		dispatcher->main(onFinish, importedTextures);
		});
}
