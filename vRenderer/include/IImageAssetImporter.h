#pragma once

#include <memory>
#include <filesystem>

#include "Texture.h"

class IImageAssetImporter
{
public:
	virtual std::shared_ptr<Texture> importTexture(std::filesystem::path textureFilePath, bool printImportData) = 0;
};