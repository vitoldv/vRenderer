#include "StbImageImporter.h"
#include "stb_image.h"

std::shared_ptr<Texture> StbImageImporter::importTexture(std::filesystem::path textureFilePath, bool printImportData)
{
	// If texture is already imported just return it
	if (importedTexturesMap.find(textureFilePath.string()) != importedTexturesMap.end())
	{
		return importedTexturesMap[textureFilePath.string()];
	}

	Texture* texture = new Texture();
	importTexture_internal(textureFilePath, printImportData, *texture);
	std::shared_ptr<Texture> texturePtr;
	texturePtr.reset(texture);

	importedTexturesMap[textureFilePath.string()] = texturePtr;

	return texturePtr;
}

std::shared_ptr<Cubemap> StbImageImporter::importCubemap(std::filesystem::path cubemapFolderPath, bool printImportData)
{
	namespace fs = std::filesystem;

	// If cubemap is already imported just return it
	if (importedCubemapsMap.find(cubemapFolderPath.string()) != importedCubemapsMap.end())
	{
		return importedCubemapsMap[cubemapFolderPath.string()];
	}

	// Assert requested cubemap.
	try
	{
		bool faces[6] = { false };
		for (auto& dirIt : fs::directory_iterator(cubemapFolderPath))
		{
			const fs::path& path = dirIt.path();

			const auto& itExt = std::find(c_supportedExtensions.begin(), c_supportedExtensions.end(), path.extension().string());
			if (itExt == c_supportedExtensions.end())
			{
				throw std::runtime_error("Image imported doesn't support provided image extension.");
			}

			const auto& itFace = std::find(c_cubemapFaces.begin(), c_cubemapFaces.end(), path.stem().string());
			if (itFace != c_cubemapFaces.end())
			{
				int index = std::distance(c_cubemapFaces.begin(), itFace);
				if (faces[index] != true)
				{
					faces[index] = true;
				}
				else
				{
					throw std::runtime_error("Duplicated face image of a cubemap is detected during import");
				}
			}
		}

		for (int i = 0; i < 6; ++i)
		{
			if (!faces[i])
			{
				throw std::runtime_error("Some of cubemap faces weren't found duting import");
			}
		}
	}
	catch (const std::exception& e)
	{
		std::string error_message = "Assertion fail on " + cubemapFolderPath.string() + "cubemap import. " + e.what();
		throw std::runtime_error(error_message);
	}

	// Import the cubemap if assertion succeded
	std::shared_ptr<Cubemap> cubemap = std::make_shared<Cubemap>();
	for (auto& dirIt : fs::directory_iterator(cubemapFolderPath))
	{
		Texture texture;
		importTexture_internal(dirIt.path(), printImportData, texture);
		if (dirIt.path().stem() == "back")
		{
			cubemap->back = std::move(texture);
		}
		else if (dirIt.path().stem() == "front")
		{
			cubemap->front = std::move(texture);
		}
		else if (dirIt.path().stem() == "top")
		{
			cubemap->top = std::move(texture);
		}
		else if (dirIt.path().stem() == "bottom")
		{
			cubemap->bottom = std::move(texture);
		}
		else if (dirIt.path().stem() == "left")
		{
			cubemap->left = std::move(texture);
		}
		else if (dirIt.path().stem() == "right")
		{
			cubemap->right = std::move(texture);
		}
	}

	importedCubemapsMap[cubemapFolderPath.string()] = cubemap;

	return cubemap;
}

void StbImageImporter::importTexture_internal(std::filesystem::path textureFilePath, bool printImportData, Texture& outTexture)
{
	int width, height, channels;
	stbi_uc* image = stbi_load(textureFilePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!image)
	{
		throw std::runtime_error("Failed to load texture \"" + textureFilePath.filename().string() + "\".");
	}

	outTexture.name = textureFilePath.stem().string();
	outTexture.ptr = static_cast<uint8_t*>(image);
	outTexture.width = static_cast<uint32_t>(width);
	outTexture.height = static_cast<uint32_t>(height);
	outTexture.size = static_cast<uint32_t>(width * height * 4);
}
