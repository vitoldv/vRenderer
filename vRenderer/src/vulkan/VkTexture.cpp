#include "VkTexture.h"

VkTexture::VkTexture(const Texture& texture, VkContext context) :
	name(texture.name)
{
	this->context = context;
	createTexture(texture);
}

VkTexture::~VkTexture()
{
	cleanup();
}

VkImageView VkTexture::getImageView() const
{
	return this->imageView;
}

void VkTexture::createTexture(const Texture& texture)
{
	createTextureImage(texture);

	imageView = createImageView(image,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, context);
}


 void VkTexture::createTextureImage(const Texture& texture)
 {
	 // Create staging buffer to hold loaded data ready to copy to device
	 VkBuffer imageStagingBuffer;
	 VkDeviceMemory imageStagingBufferMemory;
	 createBuffer(context.physicalDevice, context.logicalDevice, texture.size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &imageStagingBuffer, &imageStagingBufferMemory);

	 // copy image data to staging buffer
	 void* data;
	 vkMapMemory(context.logicalDevice, imageStagingBufferMemory, 0, texture.size, 0, &data);
	 memcpy(data, texture.ptr, static_cast<size_t>(texture.size));
	 vkUnmapMemory(context.logicalDevice, imageStagingBufferMemory);

	 // Create image to hold final texture
	 image = createImage(texture.width, texture.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		 VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &imageMemory, context);

	 // COPY IMAGE DATA
	 // transition image to be DST for copy operation
	 transitionImageLayout(context.logicalDevice, context.graphicsQueue, context.graphicsCommandPool,
		 image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	 copyImageBuffer(context.logicalDevice, context.graphicsQueue, context.graphicsCommandPool, imageStagingBuffer, image, texture.width, texture.height);

	 // transition image to be shader readable for shader usage
	 transitionImageLayout(context.logicalDevice, context.graphicsQueue, context.graphicsCommandPool,
		 image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	 // Destory staging buffers
	 vkDestroyBuffer(context.logicalDevice, imageStagingBuffer, nullptr);
	 vkFreeMemory(context.logicalDevice, imageStagingBufferMemory, nullptr);
 }

 void VkTexture::cleanup()
 {
	 vkDestroyImageView(context.logicalDevice, imageView, nullptr);
	 vkDestroyImage(context.logicalDevice, image, nullptr);
	 vkFreeMemory(context.logicalDevice, imageMemory, nullptr);
 }

