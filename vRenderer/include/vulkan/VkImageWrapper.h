#pragma once

#include <vector>
#include "VulkanUtils.h"

using namespace VkUtils;

/// <summary>
/// Wrapper for an image used cotinuously within swapchain
/// </summary>
class VkImageWrapper
{
public:
	const uint32_t imageCount;
	const VkFormat format;
	const VkExtent2D extent;
	const VkImageTiling tiling;
	const VkImageUsageFlags userFlags;
	const VkImageAspectFlags aspectFlags;
	const VkMemoryPropertyFlags propertyFlags;

	VkImageWrapper(VkFormat format, VkExtent2D extent, uint32_t imageCount,
		VkImageTiling tiling, VkImageUsageFlags userFlags, VkMemoryPropertyFlags propertyFlags,
		VkImageAspectFlags aspectFlags, VkContext context);
	~VkImageWrapper();

	const VkImageView getImageView(uint32_t index) const;

	void cleanup();

private:

	std::vector<VkImage> image;
	std::vector<VkDeviceMemory> memory;
	std::vector<VkImageView> imageView;

	VkContext context;

	void create();
};



