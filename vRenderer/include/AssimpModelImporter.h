#pragma once

#include <unordered_map>

#include "IModelAssetImporter.h"
#include "geometry_settings.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

class AssimpModelImporter : public IModelAssetImporter
{
public:
	std::shared_ptr<Model> importModel(std::filesystem::path modelFilePath, IImageAssetImporter& imageImporter, bool printImportData = false) override;

private:
	std::unordered_map<std::string, std::shared_ptr<Model>> importedModelsMap;
};
