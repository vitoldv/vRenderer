#include "VkCubemap.h"

VkCubemap::VkCubemap(const Cubemap& cubemap, VkContext context)
{
	this->context = context;
	createCubemapImage(cubemap);
}

VkCubemap::~VkCubemap()
{
	//cleanup();
}

VkImageView VkCubemap::getImageView() const
{
	return this->cubemapImageView;
}

void VkCubemap::createCubemapImage(const Cubemap& cubemap)
{
	// Create staging buffer to hold loaded data ready to copy to device
	VkBuffer imageStagingBuffer;
	VkDeviceMemory imageStagingBufferMemory;

	createBuffer(context.physicalDevice, context.logicalDevice, cubemap.getTotalSize(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &imageStagingBuffer, &imageStagingBufferMemory);

	// copy image data to staging buffer
	void* data;
	vkMapMemory(context.logicalDevice, imageStagingBufferMemory, 0, cubemap.getTotalSize(), 0, &data);
	cubemap.copyTextureData(data);
	vkUnmapMemory(context.logicalDevice, imageStagingBufferMemory);

	// Create image to hold cubemap data
	uint32_t width, height;
	cubemap.getFaceExtent(width, height);
	cubemapImage = createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &cubemapMemory, context,
		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, 6);

	// COPY IMAGE DATA
	// transition image to be DST for copy operation
	transitionImageLayout(context.logicalDevice, context.graphicsQueue, context.graphicsCommandPool,
		cubemapImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 6);
	copyImageBuffer(context.logicalDevice, context.graphicsQueue, context.graphicsCommandPool, imageStagingBuffer, cubemapImage, width, height, 6);

	// transition image to be shader readable for shader usage
	transitionImageLayout(context.logicalDevice, context.graphicsQueue, context.graphicsCommandPool,
		cubemapImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 6);

	// Destroy staging buffers
	vkDestroyBuffer(context.logicalDevice, imageStagingBuffer, nullptr);
	vkFreeMemory(context.logicalDevice, imageStagingBufferMemory, nullptr);

	cubemapImageView = createImageView(cubemapImage,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, context, VK_IMAGE_VIEW_TYPE_CUBE, 6);
}

void VkCubemap::cleanup()
{
	vkDestroyImageView(context.logicalDevice, cubemapImageView, nullptr);
	vkDestroyImage(context.logicalDevice, cubemapImage, nullptr);
	vkFreeMemory(context.logicalDevice, cubemapMemory, nullptr);
}

