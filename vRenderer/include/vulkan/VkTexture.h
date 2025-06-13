#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include "VulkanUtils.h"

#include "Texture.h"

using namespace VkUtils;

class VkTexture
{
public:

	const std::string name;

	VkTexture(const Texture& texture, VkContext context);
	~VkTexture();

	VkImageView getImageView() const;

private:
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;

	VkContext context;

	void createTexture(const Texture& texture);
	void createTextureImage(const Texture& texture);

	void cleanup();
};