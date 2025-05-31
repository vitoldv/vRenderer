#include "VkImageWrapper.h"

VkImageWrapper::VkImageWrapper(VkFormat format, VkExtent2D extent, uint32_t imageCount,
	VkImageTiling tiling, VkImageUsageFlags userFlags, VkMemoryPropertyFlags propertyFlags,
	VkImageAspectFlags aspectFlags, VkContext context) :
	imageCount(imageCount),
	format(format),
	extent(extent),
	tiling(tiling),
	userFlags(userFlags),
	propertyFlags(propertyFlags),
	aspectFlags(aspectFlags)
{
	this->context = context;
	image.resize(imageCount);
	imageView.resize(imageCount);
	memory.resize(imageCount);
	create();
}

VkImageWrapper::~VkImageWrapper()
{
	//cleanup();
}

const VkImageView VkImageWrapper::getImageView(uint32_t index) const
{
	return imageView[index];
}

void VkImageWrapper::create()
{
	for (int i = 0; i < imageCount; i++)
	{
		// Create color buffer image
		image[i] = createImage(extent.width, extent.height, format, tiling, userFlags, propertyFlags, &memory[i], context);
		imageView[i] = VkUtils::createImageView(image[i], format, aspectFlags, context);
	}
}

void VkImageWrapper::cleanup()
{
	for (int i = 0; i < imageCount; i++)
	{
		vkDestroyImageView(context.logicalDevice, imageView[i], nullptr);
		vkDestroyImage(context.logicalDevice, image[i], nullptr);
		vkFreeMemory(context.logicalDevice, memory[i], nullptr);
	}
}
