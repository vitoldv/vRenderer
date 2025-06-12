#pragma once

#include <memory>
#include <filesystem>

#include "Model.h"

class IModelAssetImporter
{
public:
	virtual std::shared_ptr<Model> importModel(std::filesystem::path modelFilePath, bool printImportData) = 0;
};