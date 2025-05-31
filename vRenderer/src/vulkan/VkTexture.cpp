#include "VkTexture.h"
#include "stb_image.h"

VkTexture::VkTexture(std::string fileName, VkContext context) :
	name(fileName.c_str())
{
	this->context = context;
	createTexture(fileName, context);
}

VkTexture::~VkTexture()
{
	cleanup();
}

VkImageView VkTexture::getImageView() const
{
	return this->imageView;
}

void VkTexture::createTexture(std::string fileName, VkContext context)
{
	createTextureImage(fileName, context);

	imageView = createImageView(image,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, context);
}

stbi_uc* loadTexture(std::string fileName, int* width, int* height, VkDeviceSize* imageSize)
{
	int channels;
	stbi_uc* image = stbi_load(fileName.c_str(), width, height, &channels, STBI_rgb_alpha);
	if (!image)
	{
		throw std::runtime_error("Failed to load texture \"" + fileName + "\".");
	}

	// calculate image size
	*imageSize = *width * *height * 4;

	return image;
}


 void VkTexture::createTextureImage(std::string fileName, VkContext context)
 {
	 int width, height;
	 VkDeviceSize imageSize;
	 stbi_uc* imageData = loadTexture(fileName, &width, &height, &imageSize);

	 // Create staging buffer to hold loaded data ready to copy to device
	 VkBuffer imageStagingBuffer;
	 VkDeviceMemory imageStagingBufferMemory;
	 createBuffer(context.physicalDevice, context.logicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &imageStagingBuffer, &imageStagingBufferMemory);

	 // copy image data to staging buffer
	 void* data;
	 vkMapMemory(context.logicalDevice, imageStagingBufferMemory, 0, imageSize, 0, &data);
	 memcpy(data, imageData, static_cast<size_t>(imageSize));
	 vkUnmapMemory(context.logicalDevice, imageStagingBufferMemory);

	 stbi_image_free(imageData);

	 // Create image to hold final texture
	 image = createImage(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		 VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &imageMemory, context);

	 // COPY IMAGE DATA
	 // transition image to be DST for copy operation
	 transitionImageLayout(context.logicalDevice, context.graphicsQueue, context.graphicsCommandPool,
		 image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	 copyImageBuffer(context.logicalDevice, context.graphicsQueue, context.graphicsCommandPool, imageStagingBuffer, image, width, height);

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

