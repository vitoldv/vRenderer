#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <fstream>
#include <iostream>
#include <array>
#include "Lighting.h"

#define VALIDATION_LAYER_OUTPUT_STR "--- VALIDATION LAYER MSG: "
#define VALIDATION_LAYER_ALLOWED_MESSAGE_SEVERITY VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
#define VALIDATION_LAYER_ALLOWED_MESSAGE_TYPE VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT

// Macro to be used on uniform struct declaration to satisfy std140 alignment
#define ALIGN_STD140 alignas(16)

// How many MODELS can be rendered
#define MAX_OBJECTS 1
#define MAX_LIGHT_SOURCES 10

const uint32_t SKYBOX_RESERVED_ID = 12314;

namespace VkUtils
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec3 normal;
		glm::vec2 uv;
	};

	struct ALIGN_STD140 UboViewProjection
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec3 viewPosition;
	};

	/// <summary>
	/// Should match the UboLight struct declared in shader.
	/// P.S. The order differs from generic light to satisfy std140 alignment
	/// with the least occupied memory. Also position and direction are present 
	/// as shading in calculated in world space
	/// </summary>
	struct ALIGN_STD140 UboLight
	{
		glm::vec4 color;
		glm::vec4 position;
		glm::vec4 direction;
		
		// 0 - none
		// 1 - directional
		// 2 - point
		// 3 - spot
		uint32_t type;

		// Attenuation
		float constant;
		float linear;
		float quadratic;
		// Spotlight components
		float cutOff;
		float outerCutOff;
		float padding[2];    // offset 72 (8 bytes of padding)

		UboLight& operator = (const Light& genericLight)
		{
			type = static_cast<uint32_t>(genericLight.type) + 1;
			color = glm::vec4(genericLight.color, 0.0f);
			direction = glm::vec4(genericLight.direction, 0.0f);		// w = 0.0 for directions (though)
			position = glm::vec4(genericLight.position, 1.0f);			// w = 1.0 for position
			constant = genericLight.constant;
			linear = genericLight.linear;
			quadratic = genericLight.quadratic;
			cutOff = glm::cos(glm::radians(genericLight.cutOff));
			outerCutOff = glm::cos(glm::radians(genericLight.outerCutOff));
			padding[0] = 0.0f;  // Initialize padding to zero
			padding[1] = 0.0f;  // Initialize padding to zero
			return *this;
		}
	};

	struct UboLightArray
	{
		UboLight lights[MAX_LIGHT_SOURCES];       // Total size: 96 * 10 = 960 bytes
	};

	struct ALIGN_STD140 UboDynamicColor
	{
		glm::vec4 color;
	};

	struct ALIGN_STD140 UboPostProcessingFeatures
	{
		float gammaCorrectionFactor;
	};

	/*
	* Push Constants provide:
	* 1. Model matrix (transform matrix)
	* 2. UseTexture flag (indicates whether mesh should be textured)
	*/
	struct PushConstant
	{
		// model matrix (transform)
		glm::mat4 model;
		// normal matrix for proper normals transformation
		// P.S. can be as well used to transform any direction vector in world space
		glm::mat4 normalMatrix;
	};

	const std::vector<glm::vec3> meshVertices = {
		{-1, -1, 0.0},	
		{1,  -1, 0.0},
		{1,  1, 0.0},
		{-1, 1, 0.0},
	};
	const std::vector<uint32_t> meshIndices = {
		1, 2, 3, 1, 3, 4
	};

	const std::vector<glm::vec2> meshTexCoords = {
		{0.0f, 1.0f},
		{1.0f, 1.0f},
		{1.0f, 0.0f},
		{0.0f, 0.0f}
	};

	// Indices (locations) of Queue Families (if they exist at all)
	struct QueueFamilyIndices
	{
		// location of Graphics queue family
		int graphicsFamily = -1;
		// location of Presentation queue family (likely to be the same as graphics family)
		int presentationFamily = -1;

		// checks if families are valid
		bool isValid()
		{
			return graphicsFamily >= 0 && presentationFamily >= 0;
		}
	};

	struct VkContext
	{
		// General
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
		VkQueue graphicsQueue;
		VkCommandPool graphicsCommandPool;
		
		// Amount of images in swapchain
		uint32_t imageCount;
		VkExtent2D imageExtent;

		VkDeviceSize minUniformBufferOffset;
	};

	struct VkSamplerDescriptorSetCreateInfo
	{
		VkDescriptorSetLayout samplerDescriptorSetLayout;
		VkSampler sampler;
	};

	struct SwapChainDetails
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities;		// surface properties like image size etc.
		std::vector<VkSurfaceFormatKHR> surfaceFormats;			// surface color formats like RGBA etc.
		std::vector<VkPresentModeKHR> presentationModes;			// specifies how images should be presented (translated) to the actual screen

		bool isValid()
		{
			return !surfaceFormats.empty() && !presentationModes.empty();
		}
	};

	struct SwapChainImage
	{
		VkImage image;
		VkImageView imageView;
	};

	static std::array<float, 4> getRGBANormalized(uint32_t color) {
		return {
			((color >> 24) & 0xFF) / 255.0f,  // R
			((color >> 16) & 0xFF) / 255.0f,  // G
			((color >> 8) & 0xFF) / 255.0f,   // B
			(color & 0xFF) / 255.0f           // A
		};
	}


	static std::vector<char> readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			throw std::runtime_error("Failed to open file.");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> fileBuffer(fileSize);

		file.seekg(0);

		file.read(fileBuffer.data(), fileSize);

		file.close();

		return fileBuffer;
	}


	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{

		std::cerr << VALIDATION_LAYER_OUTPUT_STR << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	static void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VALIDATION_LAYER_ALLOWED_MESSAGE_SEVERITY;
		createInfo.messageType = VALIDATION_LAYER_ALLOWED_MESSAGE_TYPE;
		createInfo.pfnUserCallback = debugCallback;
	}

	static uint32_t findMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t allowedTypes, VkMemoryPropertyFlags properties)
	{
		// Get preperties of physical device memory
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((allowedTypes & (1 << i))														// Index of memory type must match correspoding bit in allowedTypes
				&& (memProperties.memoryTypes[i].propertyFlags & properties) == properties)		// Desired property bit flags are part of memory type's property flags 	
			{
				// This memory type is valid, so return its index
				return i;
			}
		}
	}

	static VkCommandBuffer beginCommandBuffer(VkDevice device, VkCommandPool commandPool)
	{
		// Command buffer to hold transfer commands
		VkCommandBuffer commandBuffer;

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		// Allocate command buffer from pool
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		// Info to begin command buffer record
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;		// We're only using the command buffer once

		// Record transfer commands
		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	static void endAndSubmitCommandBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer)
	{
		// End commands
		vkEndCommandBuffer(commandBuffer);

		// submit to queue
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		// Submit transfer command to transfer queue and wait until it finishes
		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(queue);

		// Free temporary command buffer
		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	static void copyBuffer(VkDevice logicalDevice, VkQueue transferQueue, VkCommandPool transferCommandPool,
		VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bufferSize)
	{
		VkCommandBuffer transferCommandBuffer = beginCommandBuffer(logicalDevice, transferCommandPool);

		// Region of data to copy from or to
		VkBufferCopy bufferCopyRegion = {};
		bufferCopyRegion.srcOffset = 0;
		bufferCopyRegion.dstOffset = 0;
		bufferCopyRegion.size = bufferSize;

		// Command to copy src buffer to dst buffer
		vkCmdCopyBuffer(transferCommandBuffer, srcBuffer, dstBuffer, 1, &bufferCopyRegion);

		endAndSubmitCommandBuffer(logicalDevice, transferCommandPool, transferQueue, transferCommandBuffer);
	}

	static void copyImageBuffer(VkDevice logicalDevice, VkQueue transferQueue, VkCommandPool transferCommandPool, VkBuffer srcBuffer,
		VkImage image, uint32_t width, uint32_t height, uint32_t layerCount = 1)
	{
		VkCommandBuffer transferCommandBuffer = beginCommandBuffer(logicalDevice, transferCommandPool);

		VkBufferImageCopy imageRegion = {};
		imageRegion.bufferOffset = 0;
		imageRegion.bufferRowLength = 0;
		imageRegion.bufferImageHeight = 0;
		imageRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageRegion.imageSubresource.mipLevel = 0;
		imageRegion.imageSubresource.baseArrayLayer = 0;
		imageRegion.imageSubresource.layerCount = layerCount;
		imageRegion.imageOffset = { 0,0,0 };
		imageRegion.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(transferCommandBuffer, srcBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageRegion);

		endAndSubmitCommandBuffer(logicalDevice, transferCommandPool, transferQueue, transferCommandBuffer);
	}

	static void createBuffer(VkPhysicalDevice physicalDevice, VkDevice logicalDevice, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsageFlags,
		VkMemoryPropertyFlags bufferProperties, VkBuffer* buffer, VkDeviceMemory* bufferMemory)
	{
		// CREATE BUFFER
		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = bufferSize;
		bufferCreateInfo.usage = bufferUsageFlags;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, buffer);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create a buffer.");
		}

		// GET BUFFER MEMORY REQUIREMENTS
		VkMemoryRequirements memReq;
		vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memReq);

		// ALLOCATE MEMORY TO BUFFER
		VkMemoryAllocateInfo memAllocInfo = {};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize = memReq.size;
		memAllocInfo.memoryTypeIndex = findMemoryTypeIndex(physicalDevice, memReq.memoryTypeBits, bufferProperties);

		// Allocate memory to VKDeviceMemory
		result = vkAllocateMemory(logicalDevice, &memAllocInfo, nullptr, bufferMemory);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate Buffer Memory.");
		}

		// Allocate memory to given buffer
		vkBindBufferMemory(logicalDevice, *buffer, *bufferMemory, 0);
	}

	static void check_vk_result(VkResult err)
	{
		if (err == VK_SUCCESS)
			return;
		fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
		if (err < 0)
			abort();
	}

	static void transitionImageLayout(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkImage image,
		VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t layerCount = 1)
	{
		VkCommandBuffer commandBuffer = beginCommandBuffer(device, commandPool);

		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.oldLayout = oldLayout;
		imageMemoryBarrier.newLayout = newLayout;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
		imageMemoryBarrier.subresourceRange.levelCount = 1;
		imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
		imageMemoryBarrier.subresourceRange.layerCount = layerCount;

		VkPipelineStageFlags srcStage;
		VkPipelineStageFlags dstStage;

		// if transitioning from new image to image ready to receive data
		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = 0;									// memory access stage transition must after
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;		// memory access stage transition must before

			srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		// if transitioning from transfer destination to shader readable
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;		// memory access stage transition must after
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;			// memory access stage transition must before

			srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		vkCmdPipelineBarrier(commandBuffer,
			srcStage, dstStage,				// pipeline stages that should match src and dst access mask specified above
			0,								// dependency flags
			0, nullptr,						// memory barrier count + data
			0, nullptr,						// buffer memory barrier count + data
			1, &imageMemoryBarrier);

		endAndSubmitCommandBuffer(device, commandPool, queue, commandBuffer);
	}

	static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkContext context,
		VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D, uint32_t layerCount = 1)
	{
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = image;
		imageViewCreateInfo.viewType = viewType;
		imageViewCreateInfo.format = format;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;	// Allows remapping of rgba components to other rgba values
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// subresources allow to view only selected part of an image
		imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;		// which aspect of image to view (COLOR_BIT for color)
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;				// Start mipmap level to view from
		imageViewCreateInfo.subresourceRange.levelCount = 1;				// number of mipmap levels to view
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;			// start array level to view from
		imageViewCreateInfo.subresourceRange.layerCount = layerCount;		// number of array layers to view

		VkImageView imageView;
		VkResult result = vkCreateImageView(context.logicalDevice, &imageViewCreateInfo, nullptr, &imageView);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create an ImageView.");
		}

		return imageView;
	}

	static VkDescriptorPool createDescriptorPool(VkDescriptorType type, uint32_t maxSets, VkDescriptorPoolCreateFlagBits flags, VkContext context)
	{
		VkDescriptorPoolSize poolSize = {};
		poolSize.type = type;
		poolSize.descriptorCount = maxSets;

		VkDescriptorPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.maxSets = maxSets;
		createInfo.poolSizeCount = 1;
		createInfo.pPoolSizes = &poolSize;
		createInfo.flags = flags;

		VkDescriptorPool pool;
		VkResult result = vkCreateDescriptorPool(context.logicalDevice, &createInfo, nullptr, &pool);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor pool.");
		}

		return pool;
	}

	static VkImage createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags userFlags, VkMemoryPropertyFlags propertyFlags, VkDeviceMemory* imageMemory,
		VkContext context, VkImageCreateFlags imageFlags = 0, uint32_t layers = 1)
	{
		// Create the image
		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.extent.width = width;
		imageCreateInfo.extent.height = height;
		imageCreateInfo.extent.depth = 1;								// 1 because there is no 3D aspect
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = layers;
		imageCreateInfo.format = format;
		imageCreateInfo.tiling = tiling;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		// layout of image data on creation
		imageCreateInfo.usage = userFlags;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;				// number of samples for multisampling
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		// whether image can be shared between queues
		imageCreateInfo.flags = imageFlags;

		VkImage image;
		VkResult result = vkCreateImage(context.logicalDevice, &imageCreateInfo, nullptr, &image);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create an image.");
		}

		// Create memory for the image

		// Get memory requirements 
		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(context.logicalDevice, image, &memReq);

		VkMemoryAllocateInfo memAllocInfo = {};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize = memReq.size;
		memAllocInfo.memoryTypeIndex = findMemoryTypeIndex(context.physicalDevice, memReq.memoryTypeBits, propertyFlags);

		result = vkAllocateMemory(context.logicalDevice, &memAllocInfo, nullptr, imageMemory);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate memory for an image.");
		}

		// connect memory to image
		vkBindImageMemory(context.logicalDevice, image, *imageMemory, 0);

		return image;
	}

	static VkDescriptorSetLayout createDescriptorSetLayout(
		uint32_t bindingCount,
		VkShaderStageFlags shaderStage,
		VkDescriptorType descriptopType,
		VkContext context)
	{
		VkDescriptorSetLayout layout;
		std::vector<VkDescriptorSetLayoutBinding> bindings(bindingCount);
		for (int i = 0; i < bindingCount; i++)
		{
			VkDescriptorSetLayoutBinding binding;
			binding.binding = i;												// bindings specified in shader
			binding.descriptorType = descriptopType;			// type of descriptor (simple uniform in this case)
			binding.descriptorCount = 1;										// number of binded values
			binding.stageFlags = shaderStage;									// specifies shader stage
			binding.pImmutableSamplers = nullptr;
			bindings[i] = binding;
		}

		// Create descriptor set layout with given bindings
		VkDescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = bindingCount;
		createInfo.pBindings = bindings.data();

		VkResult result = vkCreateDescriptorSetLayout(context.logicalDevice, &createInfo, nullptr, &layout);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout.");
		}

		return layout;
	}


	static VkShaderModule createShaderModule(const std::vector<char>& code, VkContext context)
	{
		VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
		shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderModuleCreateInfo.codeSize = code.size();
		shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		VkResult result = vkCreateShaderModule(context.logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module.");
		}

		return shaderModule;
	}
}