#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanUtils.h"
#include "Texture.h"

using namespace VkUtils;

class VkCubemap
{
public:

	VkCubemap(const Cubemap& cubemap, VkContext context);
	~VkCubemap();

	VkImageView getImageView() const;
	void cleanup();

private:
	VkImage cubemapImage;
	VkDeviceMemory cubemapMemory;
	VkImageView cubemapImageView;
	VkContext context;

	void createCubemapImage(const Cubemap& cubemap);

};