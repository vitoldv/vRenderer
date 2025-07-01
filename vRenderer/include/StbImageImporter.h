#pragma once

#include <unordered_map>

#include "IImageAssetImporter.h"

class StbImageImporter : public IImageAssetImporter
{
public:
	std::shared_ptr<Texture> importTexture(std::filesystem::path textureFilePath, bool printImportData) override;
	std::shared_ptr<Cubemap> importCubemap(std::filesystem::path cubemapFolderPath, bool printImportData) override;

private:
	std::unordered_map<std::string, std::shared_ptr<Texture>> importedTexturesMap;
	std::unordered_map<std::string, std::shared_ptr<Cubemap>> importedCubemapsMap;

	void importTexture_internal(std::filesystem::path textureFilePath, bool printImportData, Texture& outTexture);
};