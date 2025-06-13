#pragma once

#include <unordered_map>

#include "IImageAssetImporter.h"

class StbImageImporter : public IImageAssetImporter
{
public:
	// Inherited via IImageAssetImporter
	std::shared_ptr<Texture> importTexture(std::filesystem::path textureFilePath, bool printImportData) override;

private:
	std::unordered_map<std::string, std::shared_ptr<Texture>> importedTexturesMap;
};