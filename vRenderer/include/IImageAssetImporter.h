#pragma once

#include <memory>
#include <filesystem>

#include "Texture.h"

const std::vector<std::string> c_supportedExtensions = { ".jpg", ".png" };
const std::vector<std::string> c_cubemapFaces = { "back", "front", "top", "bottom", "left", "right"};

class IImageAssetImporter
{
public:
	virtual std::shared_ptr<Texture> importTexture(std::filesystem::path textureFilePath, bool printImportData) = 0;
	virtual std::shared_ptr<Cubemap> importCubemap(std::filesystem::path cubemapFolderPath, bool printImportData) = 0;
};