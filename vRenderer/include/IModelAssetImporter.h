#pragma once

#include <memory>
#include <filesystem>

#include "Model.h"
#include "IImageAssetImporter.h"

class IModelAssetImporter
{
public:
	virtual std::shared_ptr<Model> importModel(std::filesystem::path modelFilePath, IImageAssetImporter& imageImporter, bool printImportData) = 0;
};