#include "StbImageImporter.h"
#include "stb_image.h"

std::shared_ptr<Texture> StbImageImporter::importTexture(std::filesystem::path textureFilePath, bool printImportData)
{
	// If model is already imported just return it
	if (importedTexturesMap.find(textureFilePath.string()) != importedTexturesMap.end())
	{
		return importedTexturesMap[textureFilePath.string()];
	}

	int width, height, channels;
	stbi_uc* image = stbi_load(textureFilePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!image)
	{
		throw std::runtime_error("Failed to load texture \"" + textureFilePath.filename().string() + "\".");
	}

	std::shared_ptr<Texture> texture = std::make_shared<Texture>();
	texture->ptr = static_cast<uint8_t*>(image);
	texture->width = static_cast<uint32_t>(width);
	texture->height = static_cast<uint32_t>(height);
	texture->size = static_cast<uint32_t>(width * height * 4);

	importedTexturesMap[textureFilePath.string()] = texture;

	return texture;
}
