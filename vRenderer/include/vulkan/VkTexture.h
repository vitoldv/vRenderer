#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include "VulkanUtils.h"

using namespace VkUtils;

class VkTexture
{
public:

	const std::string name;

	VkTexture(std::string fileName, VkContext context);
	~VkTexture();

	VkImageView getImageView() const;
	VkDescriptorSet createTextureSamplerDescriptor(VkSamplerDescriptorSetCreateInfo createInfo);

private:
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;

	VkContext context;

	void createTexture(std::string fileName, VkContext context);
	void createTextureImage(std::string fileName, VkContext context);

	void cleanup();
};