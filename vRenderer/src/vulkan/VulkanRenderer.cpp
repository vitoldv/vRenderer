#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer()
{
}

int VulkanRenderer::init(GLFWwindow* window)
{
	this->window = window;
	try
	{
		createVulkanInstance();
		setupDebugMessenger();
		createSurface();
		retrievePhysicalDevice();
		printPhysicalDeviceInfo(this->vkPhysicalDevice);
		createLogicalDevice();
		createSwapChain();
		createDepthBuffer();
		createColorBufferImage();
		createRenderPass();
		createDescriptorSetLayout();
		createTextureSampler();
		createPushConstantRange();
		createDescriptorPools();
		createUniforms();
		createInputDescriptorSets();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createCommandBuffers();

		createSyncTools();

		createImguiDescriptorPool();
		setupImgui();

	}
	catch (const std::runtime_error &e)
	{
		printf("ERROR: %s\n", e.what());
		return EXIT_FAILURE;
	}

	return 0;
}

void VulkanRenderer::cleanup()
{
	// Wait until there is nothing on a queue 
	vkDeviceWaitIdle(this->vkLogicalDevice);

	cleanupImgui();

	// Cleanup models
	for (auto& model : modelsToRender)
	{
		delete model;
		model = nullptr;
	}

	this->modelsToRender.clear();

	vkDestroyDescriptorPool(this->vkLogicalDevice, this->vkInputDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(this->vkLogicalDevice, this->vkInputDescriptorSetLayout , nullptr);

	vkDestroyDescriptorSetLayout(this->vkLogicalDevice, this->vkSamplerDescriptorSetLayout, nullptr); 
	vkDestroySampler(this->vkLogicalDevice, this->vkTextureSampler, nullptr);
	 
	for (int i = 0; i < IMAGE_COUNT; i++)
	{
		vkDestroyImageView(this->vkLogicalDevice, this->colorBufferImageView[i], nullptr);
		vkDestroyImage(this->vkLogicalDevice, this->colorBufferImage[i], nullptr);
		vkFreeMemory(this->vkLogicalDevice, colorBufferImageMemory[i], nullptr);
	}

	for (int i = 0; i < IMAGE_COUNT; i++)
	{
		vkDestroyImageView(this->vkLogicalDevice, this->depthBufferImageView[i], nullptr);
		vkDestroyImage(this->vkLogicalDevice, this->depthBufferImage[i], nullptr);
		vkFreeMemory(this->vkLogicalDevice, depthBufferImageMemory[i], nullptr);
	}

	for (int i = 0; i < IMAGE_COUNT; i++)
	{
		vpUniforms[i]->cleanup();
		lightUniforms[i]->cleanup();
		colorUniformsDynamic[i]->cleanup();
	}

	vkDestroyDescriptorPool(this->vkLogicalDevice, this->uniformDescriptorPool, nullptr);
	vkDestroyDescriptorPool(this->vkLogicalDevice, this->dynamicUniformDescriptorPool, nullptr);

	for (int i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		vkDestroySemaphore(this->vkLogicalDevice, this->vkSemRenderFinished[i], nullptr);
		vkDestroySemaphore(this->vkLogicalDevice, this->vkSemImageAvailable[i], nullptr);
		vkDestroyFence(this->vkLogicalDevice, this->vkDrawFences[i], nullptr);
	}

	vkDestroyCommandPool(this->vkLogicalDevice, vkGraphicsCommandPool, nullptr);
	for (auto framebuffer : vkSwapchainFramebuffers)
	{
		vkDestroyFramebuffer(this->vkLogicalDevice, framebuffer, nullptr);
	}
	
	vkDestroyPipeline(this->vkLogicalDevice, this->vkSecondPipeline, nullptr);
	vkDestroyPipelineLayout(this->vkLogicalDevice, this->vkSecondPipelineLayout, nullptr);
	vkDestroyPipeline(this->vkLogicalDevice, this->vkGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(this->vkLogicalDevice, this->vkPipelineLayout, nullptr);
	vkDestroyRenderPass(this->vkLogicalDevice, this->vkRenderPass, nullptr);
	for (auto image : swapchainImages)
	{
		vkDestroyImageView(this->vkLogicalDevice, image.imageView, nullptr);
	}
	vkDestroySwapchainKHR(this->vkLogicalDevice, this->vkSwapchain, nullptr);
	vkDestroySurfaceKHR(this->vkInstance, this->vkSurface, nullptr);

#ifndef NDEBUG
	if (ENABLE_VALIDATION_LAYERS)
	{
		VkUtils::destroyDebugUtilsMessengerEXT(this->vkInstance, this->debugMessenger, nullptr);
	}
#endif
	vkDestroyDevice(this->vkLogicalDevice, nullptr);
	vkDestroyInstance(this->vkInstance, nullptr);
}

void VulkanRenderer::setImguiCallback(std::function<void()> imguiCode)
{
	this->imguiCallback = imguiCode;
}

VulkanRenderer::~VulkanRenderer()
{
	cleanup();
}

/// <summary>
/// Creates Vulkan Instance
/// </summary>
void VulkanRenderer::createVulkanInstance()
{
	// Create Info about the application (required for VInstanceCreateInfo)
	// P.S. Most data here doesn't affect the program itself but is helpful for developers
	VkApplicationInfo vkAppInfo = {};
	vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkAppInfo.pApplicationName = "Vulkan Renderer";
	vkAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	vkAppInfo.pEngineName = "VEngine";
	vkAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	vkAppInfo.apiVersion = VK_API_VERSION_1_4;

	// Create Info for Vulkan Instance creation
	VkInstanceCreateInfo vkInstanceInfo = {};
	vkInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceInfo.pApplicationInfo = &vkAppInfo;

	// Get required Vulkan extensions
	std::vector<const char*> instanceExtensions = std::vector<const char*>();
	uint32_t extensionsCount = 0;
	const char** extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
	for (int i = 0; i < extensionsCount; i++)
	{
		instanceExtensions.push_back(extensions[i]);
	}
	if (ENABLE_VALIDATION_LAYERS)
	{
		instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	vkInstanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
	vkInstanceInfo.ppEnabledExtensionNames = instanceExtensions.data();

	// Checking whether extensions required by GLFW are supported by Vulkan Instance
	if (!isInstanceExtensionsSupported(&instanceExtensions))
	{
		throw std::runtime_error("Vulkan Instance doesn't support some extensions required by GLFW.");
	}

	// Setting Validation layers
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
	if (ENABLE_VALIDATION_LAYERS && !checkValidationLayerSupport())
	{
		throw std::runtime_error("Requested validations layers are not supported.");
	}
	if (ENABLE_VALIDATION_LAYERS)
	{
		vkInstanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		vkInstanceInfo.ppEnabledLayerNames = validationLayers.data();
		
		// This is the way to enable debug messanger for Vulkan instance creation and destroy functions
		// (messanger itself is created and destroyed in between those)
		VkUtils::populateDebugMessengerCreateInfo(debugCreateInfo);
		vkInstanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		vkInstanceInfo.enabledLayerCount = 0;
		vkInstanceInfo.ppEnabledLayerNames = nullptr;
	}
	// Create the Vulkan Instance
	VkResult result = vkCreateInstance(&vkInstanceInfo, nullptr, &this->vkInstance);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Vulkan Instance.");
	}
}

void VulkanRenderer::retrievePhysicalDevice()
{
	// Enumerate Vulkan accessible physical devices (GPUs)
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, nullptr);

	// Get the list of supported physical devices
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, devices.data());
	
	// TEMP: Simply pick the first device (preferrably a choice must be provided)
	if (isDeviceSuitable(devices[0]))
	{
		this->vkPhysicalDevice = devices[0];
	}

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(this->vkPhysicalDevice, &deviceProperties);
	
	context.minUniformBufferOffset = deviceProperties.limits.minUniformBufferOffsetAlignment;
	
	context.physicalDevice = vkPhysicalDevice;
}

void VulkanRenderer::createLogicalDevice()
{
	// Get queue family indices for selected physical device
	VkUtils::QueueFamilyIndices indices = getQueueFamilies(this->vkPhysicalDevice);

	// Creating queue family info for queue family creation (considering that some indices may point
	// out to the same queue family so we create infos for only distinct ones ensuring using set)
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> queueFamilyIndices = { indices.graphicsFamily, indices.presentationFamily };
	for (int queueFamilyIndex : queueFamilyIndices)
	{
		// Create info about Queue family logical device needs 
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
		queueCreateInfo.queueCount = 1;
		float priority = 1.0f;								// priority required for Vulkan to properly handle multiple queue families
		queueCreateInfo.pQueuePriorities = &priority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Information to create a Logical Device ("Device" shortened)
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());	// the number of Logical Devices Extensions (not the same extensions as ones for Vulkan Instance!)
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	VkPhysicalDeviceFeatures deviceFeatures = { };
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	// Physical Devices features that Logical Device is going to use
	// TEMP: Empty (default) for now
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	VkPhysicalDeviceRobustness2FeaturesEXT vulkanRobustness2Features = {};
	vulkanRobustness2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
	vulkanRobustness2Features.nullDescriptor = VK_TRUE;
	deviceCreateInfo.pNext = &vulkanRobustness2Features;

	VkResult result = vkCreateDevice(this->vkPhysicalDevice, &deviceCreateInfo, nullptr, &this->vkLogicalDevice);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Logical Device.");
	}

	// Save queues as they are created at the same time as the logical device
	vkGetDeviceQueue(this->vkLogicalDevice, indices.graphicsFamily, 0, &this->vkGraphicsQueue);
	vkGetDeviceQueue(this->vkLogicalDevice, indices.presentationFamily, 0, &this->vkPresentationQueue);

	context.logicalDevice = vkLogicalDevice;
	context.graphicsQueue = vkGraphicsQueue;
}

void VulkanRenderer::createSurface()
{
	if (glfwCreateWindowSurface(this->vkInstance, this->window, nullptr, &this->vkSurface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface.");
	}

}

void VulkanRenderer::createSwapChain()
{
	// Get Swap Chain details so we can pick best settings
	VkUtils::SwapChainDetails swapChainDetails = getSwapChainDetails(this->vkPhysicalDevice);

	// Find optimal surface values for our swap chain
	VkSurfaceFormatKHR surfaceFormat = defineSurfaceFormat(swapChainDetails.surfaceFormats);
	VkPresentModeKHR presentMode = definePresentationMode(swapChainDetails.presentationModes);
	VkExtent2D extent = defineSwapChainExtent(swapChainDetails.surfaceCapabilities);

	if (IMAGE_COUNT < swapChainDetails.surfaceCapabilities.minImageCount ||
		IMAGE_COUNT > swapChainDetails.surfaceCapabilities.maxImageCount)
	{
		throw std::runtime_error("Specified IMAGE_COUNT is not possible on this device.");
	}

	// Creation information for swap chain
	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = this->vkSurface;												// Swapchain surface
	swapChainCreateInfo.imageFormat = surfaceFormat.format;										// Swapchain format
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;								// Swapchain color space
	swapChainCreateInfo.presentMode = presentMode;												// Swapchain presentation mode
	swapChainCreateInfo.imageExtent = extent;													// Swapchain image extents
	swapChainCreateInfo.minImageCount = IMAGE_COUNT;												// Minimum images in swapchain
	swapChainCreateInfo.imageArrayLayers = 1;													// Number of layers for each image in chain
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;						// What attachment images will be used as
	swapChainCreateInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform;	// Transform to perform on swap chain images
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;						// How to handle blending images with external graphics (e.g. other windows)
	swapChainCreateInfo.clipped = VK_TRUE;														// Whether to clip parts of image not in view (e.g. behind another window, off screen, etc)

	// Get Queue Family Indices
	VkUtils::QueueFamilyIndices indices = getQueueFamilies(this->vkPhysicalDevice);

	// If Graphics and Presentation families are different, then swapchain must let images be shared between families
	if (indices.graphicsFamily != indices.presentationFamily)
	{
		// Queues to share between
		uint32_t queueFamilyIndices[] = {
			(uint32_t)indices.graphicsFamily,
			(uint32_t)indices.presentationFamily
		};

		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;		// Image share handling
		swapChainCreateInfo.queueFamilyIndexCount = 2;							// Number of queues to share images between
		swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;			// Array of queues to share between
	}
	else
	{
		swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapChainCreateInfo.queueFamilyIndexCount = 0;
		swapChainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	// IF old swap chain been destroyed and this one replaces it, then link old one to quickly hand over responsibilities
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create Swapchain
	VkResult result = vkCreateSwapchainKHR(this->vkLogicalDevice, &swapChainCreateInfo, nullptr, &this->vkSwapchain);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Swapchain!");
	}

	// Store for later reference
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	// Get swap chain images (first count, then values)
	uint32_t swapChainImageCount;
	vkGetSwapchainImagesKHR(this->vkLogicalDevice, this->vkSwapchain, &swapChainImageCount, nullptr);
	std::vector<VkImage> images(swapChainImageCount);
	vkGetSwapchainImagesKHR(this->vkLogicalDevice, this->vkSwapchain, &swapChainImageCount, images.data());

	for (VkImage image : images)
	{
		// Store image handle
		VkUtils::SwapChainImage swapChainImage = {};
		swapChainImage.image = image;
		swapChainImage.imageView = VkUtils::createImageView(image, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, context);

		// Add to swapchain image list
		this->swapchainImages.push_back(swapChainImage);
	}
}

void VulkanRenderer::createRenderPass()
{
	// Array of subpasses (of information about a particular subpass the Render Pass is using)
	std::array<VkSubpassDescription, 2> subpasses = {};

	// ----- SUBPASS 1 - INPUT ATTACHMENTS -----

	// Color attachment (Input)
	VkAttachmentDescription colorAttachment = {};
 	colorAttachment.format = this->colorFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// clear attachment on render pass load
	// don't care what to do with attachmnet after render pass finished
	// because at that moment only swapchain output matters
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;		
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth attachment (Input)
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = depthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// References
	// Color attachment reference
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 1;												// should match an index of corresponding image in framebuffer	
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	// Depth attachment reference
	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 2;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// SUBPASS 1 INFO
	subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;				// Pipeline type subpass is to be bound to
	subpasses[0].colorAttachmentCount = 1;
	subpasses[0].pColorAttachments = &colorAttachmentRef;
	subpasses[0].pDepthStencilAttachment = &depthAttachmentRef;

	// ----- SUBPASS 2 ATTACHMENTS -----

	// Swapchain color attachment (output)
	VkAttachmentDescription swapchainColorAttachment = {};
	swapchainColorAttachment.format = swapChainImageFormat;						// Format to use for attachment
	swapchainColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;					// Number of samples to write for multisampling
	swapchainColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;				// Describes what to do with attachment before rendering
	swapchainColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;				// Describes what to do with attachment after rendering
	swapchainColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// Describes what to do with stencil before rendering
	swapchainColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;	// Describes what to do with stencil after rendering
	// Framebuffer data will be stored as an image, but images can be given different data layouts
	// to give optimal use for certain operations
	swapchainColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// Image data layout before render pass starts
	swapchainColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		// Image data layout after render pass (to change to)

	VkAttachmentReference swapchainColorAttachmentRef = {};
	swapchainColorAttachmentRef.attachment = 0;
	swapchainColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// References to attachments that subpass will take input from (attachments subpass 1 was writing to)
	std::array<VkAttachmentReference, 2> inputReferences = {};
	inputReferences[0].attachment = 1;
	inputReferences[0].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	inputReferences[1].attachment = 2;
	inputReferences[1].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// SUBPASS 2 INFO
	subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;				// Pipeline type subpass is to be bound to
	subpasses[1].colorAttachmentCount = 1;
	subpasses[1].pColorAttachments = &swapchainColorAttachmentRef;
	subpasses[1].inputAttachmentCount = static_cast<uint32_t>(inputReferences.size());
	subpasses[1].pInputAttachments = inputReferences.data();

	// ----- SUBPASS DEPENDENCIES -----

	// Need to determine when layout transitions occur using subpass dependencies
	std::array<VkSubpassDependency, 3> subpassDependencies;

	// Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// Transition must happen after...
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;						// Subpass index (VK_SUBPASS_EXTERNAL = Special value meaning outside of renderpass)
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;		// Pipeline stage
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;				// Stage access mask (memory access)
	// But must happen before...
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = 0;

	// Subpass 1 layout (color/depth) to subpass 2 layout (shader read)
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstSubpass = 1;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	subpassDependencies[1].dependencyFlags = 0;

	// Conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// Transition must happen after...
	subpassDependencies[2].srcSubpass = 0;
	subpassDependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;;
	// But must happen before...
	subpassDependencies[2].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[2].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[2].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[2].dependencyFlags = 0;

	std::array<VkAttachmentDescription, 3> renderPassAttachments = 
	{ swapchainColorAttachment, colorAttachment, depthAttachment };

	// Create info for Render Pass
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(renderPassAttachments.size());
	renderPassCreateInfo.pAttachments = renderPassAttachments.data();
	renderPassCreateInfo.subpassCount = static_cast<uint32_t>(subpasses.size());
	renderPassCreateInfo.pSubpasses = subpasses.data();
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	VkResult result = vkCreateRenderPass(this->vkLogicalDevice, &renderPassCreateInfo, nullptr, &this->vkRenderPass);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Render Pass.");
	}
}

void VulkanRenderer::createGraphicsPipeline()
{
	///////////////////////////////////
	// ----- FIRST PASS PIPELINE -----
	///////////////////////////////////

	// SHADER STAGES SETUP

	// build shader modules to link to graphics pipeline
	VkShaderModule vertexShaderModule = createShaderModule(VkUtils::readFile("vRenderer\\shaders\\vulkan\\first_pass_vert.spv"));
	VkShaderModule fragmentShaderModule = createShaderModule(VkUtils::readFile("vRenderer\\shaders\\vulkan\\first_pass_frag.spv"));

	// VERTEX STAGE CREATION
	VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
	vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;			// shader stage name
	vertexShaderStageCreateInfo.module = vertexShaderModule;				// shader module to be used
	vertexShaderStageCreateInfo.pName = "main";								// shader enter function

	// FRAGMENT STAGE CREATION
	VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
	fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;		// shader stage name
	fragmentShaderStageCreateInfo.module = fragmentShaderModule;				// shader module to be used
	fragmentShaderStageCreateInfo.pName = "main";								// shader enter function

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

	// DEFINING VERTEX ATTRIBUTES LAYOUT
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::array<VkVertexInputAttributeDescription, 4> attributes;
		attributes[0].binding = 0;										// should be same as above
		attributes[0].location = 0;
		attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[0].offset = offsetof(Vertex, pos);

		attributes[1].binding = 0;										// should be same as above
		attributes[1].location = 1;
		attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[1].offset = offsetof(Vertex, color);

		attributes[2].binding = 0;										// should be same as above
		attributes[2].location = 2;
		attributes[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[2].offset = offsetof(Vertex, normal);

		attributes[3].binding = 0;										// should be same as above
		attributes[3].location = 3;
		attributes[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributes[3].offset = offsetof(Vertex, uv);

		// VERTEX INPUT
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
		vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;					// list of vertex binding descriptions (data spacing/stride information)
		vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
		vertexInputCreateInfo.pVertexAttributeDescriptions = attributes.data();				// list of vertex attribute descriptions (data format and where to bind to/from)
	}

	// INPUT ASSEMBLY
	// defines how vertex data is perceived (topology)
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	{
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
	}

	// VIEWPORT & SCISSORS
	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	{
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)this->swapChainExtent.width;
		viewport.height = (float)this->swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0,0 };
		scissor.extent = this->swapChainExtent;

		viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCreateInfo.viewportCount = 1;
		viewportStateCreateInfo.pViewports = &viewport;
		viewportStateCreateInfo.scissorCount = 1;
		viewportStateCreateInfo.pScissors = &scissor;
	}

	/*
	// DYNAMIC STATES
	// Dynamic states to enable
	std::vector<VkDynamicState> dynamicStatesEnables;
	dynamicStatesEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);	// allows to change viewport on runtime using vkCmdSetViewport
	dynamicStatesEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStatesEnables.size());
	dynamicStateCreateInfo.pDynamicStates = dynamicStatesEnables.data();
	*/

	// RASTERIZER
	VkPipelineRasterizationStateCreateInfo rastCreateInfo = {};
	{
		rastCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rastCreateInfo.depthClampEnable = VK_FALSE;
		rastCreateInfo.rasterizerDiscardEnable = VK_FALSE;
		rastCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rastCreateInfo.lineWidth = 1.0f;
		rastCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rastCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rastCreateInfo.depthBiasEnable = VK_FALSE;				// whether to add depth bias to fragments (good for stopping "shadow acne" in shadow mapping)
	}

	// MULTI SAMPLING
	VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
	{
		multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
		multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	}

	// COLOR SETUP DEFINITION (blending etc.)
	VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};
	{
		// BLENDING
		// Blending decides how to blend a new color being written to a fragment, with the old value

		// Blend Attachment State (how blending is handled)
		VkPipelineColorBlendAttachmentState colorState = {};
		colorState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT	// colors to apply blending to
			| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorState.blendEnable = VK_TRUE;													// Enable blending

		// Blending uses equation: (srcColorBlendFactor * new color) colorBlendOp (dstColorBlendFactor * old color)
		colorState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorState.colorBlendOp = VK_BLEND_OP_ADD;

		// Summarised: (VK_BLEND_FACTOR_SRC_ALPHA * new color) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * old color)
		//			   (new color alpha * new color) + ((1 - new color alpha) * old color)

		colorState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorState.alphaBlendOp = VK_BLEND_OP_ADD;
		// Summarised: (1 * new alpha) + (0 * old alpha) = new alpha

		colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendingCreateInfo.logicOpEnable = VK_FALSE;
		colorBlendingCreateInfo.attachmentCount = 1;
		colorBlendingCreateInfo.pAttachments = &colorState;
	}

	// PIPELINE LAYOUT SETUP (DESCRIPTORS AND PUSH CONSTANTS LAYOUT)
	{
		// Each layout should correspond to the Descriptor Set type of the same index in shader
		// Example: layout(set = 0) == layout at index 0 here
		std::array<VkDescriptorSetLayout, 4> descriptorSetLayouts = {
			this->vpUniforms[0]->getDescriptorLayout(),
			this->vkSamplerDescriptorSetLayout,
			this->lightUniforms[0]->getDescriptorLayout(),
			this->colorUniformsDynamic[0]->getDescriptorLayout()
		};

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
		pipelineLayoutCreateInfo.pPushConstantRanges = &this->vkPushConstantRange;

		// Create Pipeline Layout
		VkResult result = vkCreatePipelineLayout(this->vkLogicalDevice, &pipelineLayoutCreateInfo, nullptr, &vkPipelineLayout);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Pipeline Layout!");
		}
	}

	// DEPTH TESTING AND STENCIL TESTING SETUP
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	{
		depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilCreateInfo.depthTestEnable = VK_TRUE;
		depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
		depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
		depthStencilCreateInfo.stencilTestEnable = VK_FALSE;
	}

	// -- GRAPHICS PIPELINE CREATION --
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;									// Number of shader stages
	pipelineCreateInfo.pStages = shaderStages;							// List of shader stages
	pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;		// All the fixed function pipeline states
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.pRasterizationState = &rastCreateInfo;
	pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
	pipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
	pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
	pipelineCreateInfo.layout = this->vkPipelineLayout;							// Pipeline Layout pipeline should use
	pipelineCreateInfo.renderPass = this->vkRenderPass;							// Render pass description the pipeline is compatible with
	pipelineCreateInfo.subpass = 0;							 			// Subpass of render pass to use with pipeline

	// Pipeline Derivatives : Can create multiple pipelines that derive from one another for optimisation
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;	// Existing pipeline to derive from...
	pipelineCreateInfo.basePipelineIndex = -1;				// or index of pipeline being created to derive from (in case creating multiple at once)

	// Create Graphics Pipeline
	VkResult result = vkCreateGraphicsPipelines(this->vkLogicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &this->vkGraphicsPipeline);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create a Graphics Pipeline!");
	}

	// Shader modules are no longer needed once pipeline is created
	vkDestroyShaderModule(this->vkLogicalDevice, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(this->vkLogicalDevice, vertexShaderModule, nullptr);

	///////////////////////////////////
	// ----- SECOND PASS PIPELINE -----
	///////////////////////////////////

	VkShaderModule secondVertexShaderModule = createShaderModule(VkUtils::readFile("vRenderer\\shaders\\vulkan\\second_pass_vert.spv"));
	VkShaderModule secondFragmentShaderModule = createShaderModule(VkUtils::readFile("vRenderer\\shaders\\vulkan\\second_pass_frag.spv"));

	// reusing shader create infos for previous pipeline
	vertexShaderStageCreateInfo.module = secondVertexShaderModule;
	fragmentShaderStageCreateInfo.module = secondFragmentShaderModule;

	VkPipelineShaderStageCreateInfo secondShaderStages[] = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

	// No vertex data for second pass
	vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
	vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
	vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
	vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

	// Don't want to write to depth buffer
	depthStencilCreateInfo.depthWriteEnable = VK_FALSE;

	// Create new pipeline layout
	VkPipelineLayoutCreateInfo secondPipelineLayoutCreateInfo = {};
	secondPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	secondPipelineLayoutCreateInfo.setLayoutCount = 1;
	secondPipelineLayoutCreateInfo.pSetLayouts = &this->vkInputDescriptorSetLayout;
	secondPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	secondPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	result = vkCreatePipelineLayout(this->vkLogicalDevice, &secondPipelineLayoutCreateInfo, nullptr, &this->vkSecondPipelineLayout);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create second pipeline layout.");
	}

	pipelineCreateInfo.pStages = secondShaderStages;
	pipelineCreateInfo.layout = this->vkSecondPipelineLayout;
	pipelineCreateInfo.subpass = 1;								// second subpass

	// Create second pipeline
	result = vkCreateGraphicsPipelines(this->vkLogicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &this->vkSecondPipeline);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create second graphics pipeline.");
	}

	vkDestroyShaderModule(this->vkLogicalDevice, secondVertexShaderModule, nullptr);
	vkDestroyShaderModule(this->vkLogicalDevice, secondFragmentShaderModule, nullptr);
}

void VulkanRenderer::createColorBufferImage()
{
	this->colorBufferImage.resize(IMAGE_COUNT);
	this->colorBufferImageView.resize(IMAGE_COUNT);
	this->colorBufferImageMemory.resize(IMAGE_COUNT);

	// Get supported format for depth buffer
	this->colorFormat = defineSupportedFormat(
		{ VK_FORMAT_R8G8B8A8_UNORM },
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	for (int i = 0; i < IMAGE_COUNT; i++)
	{
		// Create color buffer image
		this->colorBufferImage[i] = VkUtils::createImage(swapChainExtent.width, swapChainExtent.height, colorFormat,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &this->colorBufferImageMemory[i], context);
		this->colorBufferImageView[i] = VkUtils::createImageView(this->colorBufferImage[i], colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, context);
	}
}

void VulkanRenderer::createFramebuffers()
{
	vkSwapchainFramebuffers.resize(IMAGE_COUNT);
	for (int i = 0; i < vkSwapchainFramebuffers.size(); i++)
	{
		std::array<VkImageView, 3> attachments = {
			swapchainImages[i].imageView,
			colorBufferImageView[i],
			depthBufferImageView[i]
		};

		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = this->vkRenderPass;
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();								// list of attachements 1:1 with render pass
		framebufferCreateInfo.width = swapChainExtent.width;
		framebufferCreateInfo.height = swapChainExtent.height;
		framebufferCreateInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(this->vkLogicalDevice, &framebufferCreateInfo, nullptr, &vkSwapchainFramebuffers[i]);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create framebuffers.");
		}
	}
}

void VulkanRenderer::createCommandPool()
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = getQueueFamilies(this->vkPhysicalDevice).graphicsFamily;
	
	VkResult result = vkCreateCommandPool(this->vkLogicalDevice, &poolInfo, nullptr, &vkGraphicsCommandPool);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Graphics Command Pool.");
	}

	context.graphicsCommandPool = vkGraphicsCommandPool;
}

void VulkanRenderer::createCommandBuffers()
{
	this->vkCommandBuffers.resize(IMAGE_COUNT);

	VkCommandBufferAllocateInfo cbAllocInfo = {};
	cbAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbAllocInfo.commandPool = vkGraphicsCommandPool;
	cbAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;					// VK_COMMAND_BUFFER_LEVEL_PRIMARY		: buffer you submit directly to the queue. Can't be called by other buffers
																			// VK_COMMAND_BUFFER_LEVEL_SECONDARY	: buffer can't be called directly. Can be called from other buffers via vkCmdExecuteCommands when recording commands in primary buffer
	cbAllocInfo.commandBufferCount = static_cast<uint32_t>(this->vkCommandBuffers.size());

	VkResult result = vkAllocateCommandBuffers(this->vkLogicalDevice, &cbAllocInfo, this->vkCommandBuffers.data());
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers");
	}
}

void VulkanRenderer::createSyncTools()
{
	this->vkSemImageAvailable.resize(MAX_FRAME_DRAWS);
	this->vkSemRenderFinished.resize(MAX_FRAME_DRAWS);
	this->vkDrawFences.resize(MAX_FRAME_DRAWS);

	VkSemaphoreCreateInfo semCreateInfo = {};
	semCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < MAX_FRAME_DRAWS; i++)
	{
		if (vkCreateSemaphore(this->vkLogicalDevice, &semCreateInfo, nullptr, &vkSemImageAvailable[i]) != VK_SUCCESS
			|| vkCreateSemaphore(this->vkLogicalDevice, &semCreateInfo, nullptr, &vkSemRenderFinished[i]) != VK_SUCCESS
			|| vkCreateFence(this->vkLogicalDevice, &fenceCreateInfo, nullptr, &this->vkDrawFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create semaphores and/or fence.");
		}
	}
}

void VulkanRenderer::createDescriptorSetLayout()
{
	// TEXTURE SAMPLER DESCRIPTOR SET LAYOUT
	{
		VkDescriptorSetLayoutBinding diffuseMapSamplerBinding = {};
		diffuseMapSamplerBinding.binding = 0;
		diffuseMapSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		diffuseMapSamplerBinding.descriptorCount = 1;
		diffuseMapSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		diffuseMapSamplerBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding specularMapSamplerBinding = {};
		specularMapSamplerBinding.binding = 1;
		specularMapSamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		specularMapSamplerBinding.descriptorCount = 1;
		specularMapSamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		specularMapSamplerBinding.pImmutableSamplers = nullptr;
		 
		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { diffuseMapSamplerBinding, specularMapSamplerBinding };

		VkDescriptorSetLayoutCreateInfo samplerDescriptorLayoutCreateInfo = {};
		samplerDescriptorLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		samplerDescriptorLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		samplerDescriptorLayoutCreateInfo.pBindings = bindings.data();

		VkResult result = vkCreateDescriptorSetLayout(this->vkLogicalDevice, &samplerDescriptorLayoutCreateInfo,
			nullptr, &this->vkSamplerDescriptorSetLayout);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Texture Samplers Descriptor Set Layout.");
		}
	}

	// INPUT ATTACHMENT IMAGE DESCRIPTOR SET LAYOUT
	{
		// Color input binding
		VkDescriptorSetLayoutBinding colorInputBinding = {};
		colorInputBinding.binding = 0;
		colorInputBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		colorInputBinding.descriptorCount = 1;
		colorInputBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		// Depth input binding
		VkDescriptorSetLayoutBinding depthInputBinding = {};
		depthInputBinding.binding = 1;
		depthInputBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		depthInputBinding.descriptorCount = 1;
		depthInputBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::vector<VkDescriptorSetLayoutBinding> inputBindings = { colorInputBinding, depthInputBinding };

		VkDescriptorSetLayoutCreateInfo inputLayoutCreateInfo = {};
		inputLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		inputLayoutCreateInfo.bindingCount = static_cast<uint32_t>(inputBindings.size());
		inputLayoutCreateInfo.pBindings = inputBindings.data();

		VkResult result = vkCreateDescriptorSetLayout(this->vkLogicalDevice, &inputLayoutCreateInfo,
			nullptr, &this->vkInputDescriptorSetLayout);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Input Descriptor Set Layout.");
		}
	}
}

void VulkanRenderer::createUniforms()
{
	vpUniforms.resize(IMAGE_COUNT);
	lightUniforms.resize(IMAGE_COUNT);
	colorUniformsDynamic.resize(IMAGE_COUNT);

	for (int i = 0; i < IMAGE_COUNT; i++)
	{
		vpUniforms[i] = std::make_unique<VkUniform<UboViewProjection>>(VK_SHADER_STAGE_VERTEX_BIT, context);
		lightUniforms[i] = std::make_unique<VkUniform<UboLightArray>>(VK_SHADER_STAGE_FRAGMENT_BIT, context);
		colorUniformsDynamic[i] = std::make_unique<VkUniformDynamic<UboDynamicColor>>(VK_SHADER_STAGE_FRAGMENT_BIT, context);
	}
}

void VulkanRenderer::createDepthBuffer()
{
	this->depthBufferImage.resize(IMAGE_COUNT);
	this->depthBufferImageView.resize(IMAGE_COUNT);
	this->depthBufferImageMemory.resize(IMAGE_COUNT);

	// Get supported format for depth buffer
	depthFormat = defineSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	for (int i = 0; i < IMAGE_COUNT; i++)
	{
		// Create depth buffer image
		this->depthBufferImage[i] = VkUtils::createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &this->depthBufferImageMemory[i], context);
		this->depthBufferImageView[i] = VkUtils::createImageView(this->depthBufferImage[i], depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, context);
	}
}   

void VulkanRenderer::createDescriptorPools()
{
	dynamicUniformDescriptorPool = VkUtils::createDescriptorPool(
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		IMAGE_COUNT,
		VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		context);
	context.dynamicUniformDescriptorPool = dynamicUniformDescriptorPool;

	// UNIFORM descriptor pool
	uniformDescriptorPool = VkUtils::createDescriptorPool(
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		3 + MAX_LIGHT_SOURCES,
		VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT, 
		context);
	context.uniformDescriptorPool = uniformDescriptorPool;

	// CREATE INPUT ATTACHMENT DESCRIPTOR POOL
	VkDescriptorPoolSize colorInputPoolSize = {};
	colorInputPoolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	colorInputPoolSize.descriptorCount = static_cast<uint32_t>(colorBufferImageView.size());
	VkDescriptorPoolSize depthInputPoolSize = {};
	depthInputPoolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	depthInputPoolSize.descriptorCount = static_cast<uint32_t>(depthBufferImageView.size());

	std::vector<VkDescriptorPoolSize> inputPoolSizes = { colorInputPoolSize, depthInputPoolSize };

	VkDescriptorPoolCreateInfo inputPoolCreateInfo = {};
	inputPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	inputPoolCreateInfo.maxSets = static_cast<uint32_t>(colorBufferImageView.size()); // can either use depthBufferImageView.size() or swapchainImages.size()
	inputPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(inputPoolSizes.size());
	inputPoolCreateInfo.pPoolSizes = inputPoolSizes.data();

	VkResult result = vkCreateDescriptorPool(this->vkLogicalDevice, &inputPoolCreateInfo, nullptr, &this->vkInputDescriptorPool);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create Input Descriptor pools.");
	}
}

void VulkanRenderer::createInputDescriptorSets()
{
	this->vkInputDescriptorSets.resize(IMAGE_COUNT);

	std::vector<VkDescriptorSetLayout> setLayouts(IMAGE_COUNT, this->vkInputDescriptorSetLayout);

	VkDescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.descriptorPool = this->vkInputDescriptorPool;
	setAllocInfo.descriptorSetCount = static_cast<uint32_t>(IMAGE_COUNT);
	setAllocInfo.pSetLayouts = setLayouts.data();

	VkResult result = vkAllocateDescriptorSets(this->vkLogicalDevice, &setAllocInfo, this->vkInputDescriptorSets.data());
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Input Attachment descriptor sets.");
	}

	for (int i = 0; i < IMAGE_COUNT; i++)
	{
		VkDescriptorImageInfo colorAttachmentDescriptor = {};
		colorAttachmentDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; 
		colorAttachmentDescriptor.imageView = colorBufferImageView[i];
		colorAttachmentDescriptor.sampler = VK_NULL_HANDLE;

		// Color attachment descriptor write
		VkWriteDescriptorSet colorWrite = {};
		colorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		colorWrite.dstSet = this->vkInputDescriptorSets[i];
		colorWrite.dstBinding = 0;
		colorWrite.dstArrayElement = 0;
		colorWrite.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		colorWrite.descriptorCount = 1;
		colorWrite.pImageInfo = &colorAttachmentDescriptor;

		VkDescriptorImageInfo depthAttachmentDescriptor = {};
		depthAttachmentDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		depthAttachmentDescriptor.imageView = depthBufferImageView[i];
		depthAttachmentDescriptor.sampler = VK_NULL_HANDLE;

		// Color attachment descriptor write
		VkWriteDescriptorSet depthWrite = {};
		depthWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		depthWrite.dstSet = this->vkInputDescriptorSets[i];
		depthWrite.dstBinding = 1;
		depthWrite.dstArrayElement = 0;
		depthWrite.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		depthWrite.descriptorCount = 1;
		depthWrite.pImageInfo = &depthAttachmentDescriptor;

		std::vector<VkWriteDescriptorSet> setWrites = { colorWrite, depthWrite };
		vkUpdateDescriptorSets(this->vkLogicalDevice, static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
	}
}

void VulkanRenderer::createPushConstantRange()
{
	// Defines push constant values
	this->vkPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	this->vkPushConstantRange.offset = 0;
	this->vkPushConstantRange.size = sizeof(PushConstant);
}
	

VkSurfaceFormatKHR VulkanRenderer::defineSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	// If only 1 format available and is undefined, then this means ALL formats are available (no restrictions)
	if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { SURFACE_COLOR_FORMAT, SURFACE_COLOR_SPACE };
	}

	// If restricted, search for optimal format
	for (const auto& format : formats)
	{
		if (format.format == SURFACE_COLOR_FORMAT && format.colorSpace == SURFACE_COLOR_SPACE)
		{
			return format;
		}
	}

	// If can't find optimal format, then just return first format
	return formats[0];
}

void VulkanRenderer::updateUniformBuffers(uint32_t imageIndex)
{
	// Update ViewProjection uniform
	{
		UboViewProjection mvp = {};
		mvp.projection = mainCamera->getProjectionMatrix();
		mvp.view = mainCamera->getViewMatrix();

		vpUniforms[imageIndex]->update(mvp);
	}

	// Update light uniform
	{
		for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
		{
			if (i < lightSources.size())
			{
				uboLightArray.lights[i] = *lightSources[i];
			}
			else
			{
				uboLightArray.lights[i].type = static_cast<Light::Type>(0);
			}
		}

		lightUniforms[imageIndex]->update(uboLightArray);
	}

	// UPDATE DYNAMIC UNIFORMS
	{
		if (modelsToRender.size() > 0)
		{
			// This is bad and should not happen every frame. TODO: remove when I know what to do with dynamic uniforms
			std::vector<UboDynamicColor> colorUbo(modelsToRender.size(), { glm::vec4(0.33f, 0.55f, 0.77f, 1.0f) });
			for (int i = 0; i < modelsToRender.size(); i++)
			{
				colorUniformsDynamic[imageIndex]->update(colorUbo.data(), modelsToRender.size());
			}
		}
	}
}

void VulkanRenderer::recordCommands(uint32_t currentImage, ImDrawData& imguiDrawData)
{
	VkCommandBufferBeginInfo bufferBeginInfo = {};
	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Information about how to begin a render pass (only required for graphical apps)
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = this->vkRenderPass;
	renderPassBeginInfo.renderArea.offset = { 0,0 };						// start point of render pass
	renderPassBeginInfo.renderArea.extent = swapChainExtent;
	
	auto backgroundColor = VkUtils::getRGBANormalized(BACKGROUND_COLOR);
	std::array<VkClearValue, 3> clearValues = {};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };		// basically it doesn't matter what will be here
	clearValues[1].color = {  backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3] };
	clearValues[2].depthStencil.depth = 1.0f;

	renderPassBeginInfo.pClearValues = clearValues.data();							// list of clear values
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());

	renderPassBeginInfo.framebuffer = this->vkSwapchainFramebuffers[currentImage];

	// Start recording commands to command buffer 
	VkResult result = vkBeginCommandBuffer(this->vkCommandBuffers[currentImage], &bufferBeginInfo);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to start recording a command buffer.");
	}

	// Begin render pass
	vkCmdBeginRenderPass(this->vkCommandBuffers[currentImage], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);;

	// bind pipeline to be used with render pass
	vkCmdBindPipeline(this->vkCommandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, this->vkGraphicsPipeline);

	// bind (static) uniforms
	vpUniforms[currentImage]->cmdBind(0, this->vkCommandBuffers[currentImage], this->vkPipelineLayout);
	// sets 1 and 2 are diffuseMap and specularMap
	lightUniforms[currentImage]->cmdBind(2, this->vkCommandBuffers[currentImage], this->vkPipelineLayout);

	int meshCount = 0;
	for (int i = 0; i < modelsToRender.size(); i++)
	{
		// bind dynamic uniforms (unique per object)
		colorUniformsDynamic[currentImage]->cmdBind(i, 3, this->vkCommandBuffers[currentImage], this->vkPipelineLayout);
		modelsToRender[i]->draw(this->vkCommandBuffers[currentImage], this->vkPipelineLayout, this->mainCamera);
	}

	// Start second subpass
	vkCmdNextSubpass(this->vkCommandBuffers[currentImage], VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(this->vkCommandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, this->vkSecondPipeline);
	vkCmdBindDescriptorSets(this->vkCommandBuffers[currentImage], VK_PIPELINE_BIND_POINT_GRAPHICS, this->vkSecondPipelineLayout,
		0, 1, &this->vkInputDescriptorSets[currentImage], 0, nullptr);
	vkCmdDraw(this->vkCommandBuffers[currentImage], 3, 1, 0, 0);

	ImGui_ImplVulkan_RenderDrawData(&imguiDrawData, this->vkCommandBuffers[currentImage]);

	// End render pass
	vkCmdEndRenderPass(this->vkCommandBuffers[currentImage]);

	// Stop recording commands to command buffer 
	result = vkEndCommandBuffer(this->vkCommandBuffers[currentImage]);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to stop recording a command buffer.");
	}
}

VkPresentModeKHR VulkanRenderer::definePresentationMode(const std::vector<VkPresentModeKHR> presentationModes)
{
	// Look for Mailbox presentation mode
	for (const auto& presentationMode : presentationModes)
	{
		if (presentationMode == SURFACE_PRESENTATION_MODE)
		{
			return presentationMode;
		}
	}

	// If can't find, use FIFO as Vulkan spec says it must be present
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::defineSwapChainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	// If current extent is at numeric limits, then extent can vary. Otherwise, it is the size of the window.
	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return surfaceCapabilities.currentExtent;
	}
	else
	{
		// If value can vary, need to set manually

		// Get window size
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		// Create new extent using window size
		VkExtent2D newExtent = {};
		newExtent.width = static_cast<uint32_t>(width);
		newExtent.height = static_cast<uint32_t>(height);

		// Surface also defines max and min, so make sure within boundaries by clamping value
		newExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
		newExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

		return newExtent;
	}
}

void VulkanRenderer::draw()
{
	ImDrawData* imguiDrawData = drawImgui();

	// 1 Get next available image to draw to and set something to signal when we're finished with the image (a semaphore)
	// 2 Submit command buffer to queue for execution,  making sure it waits for the image to e signalled as available before drawing
	// and signals when it ahas finished rendering
	// 3 Present image to screen when it has signalled finished rendering

	// Wait for given fence to signal open from last draw before continuing
	vkWaitForFences(this->vkLogicalDevice, 1, &vkDrawFences[currentFrame], VK_TRUE, std::numeric_limits<InputState>::max());
	// Reset fence
	vkResetFences(this->vkLogicalDevice, 1, &vkDrawFences[currentFrame]);

	if (!modelsToDestroy.empty())
	{
		for (auto* model : modelsToDestroy)
		{
			delete model;
			model = nullptr;
		}
		modelsToDestroy.clear();
	}

	// -- 1
	uint32_t imageIndex;
	vkAcquireNextImageKHR(this->vkLogicalDevice, this->vkSwapchain, std::numeric_limits<InputState>::max(), this->vkSemImageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

	updateUniformBuffers(imageIndex);
	recordCommands(imageIndex, *imguiDrawData);
	
	// -- 2
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;									// num of semaphores to wait for
	submitInfo.pWaitSemaphores = &vkSemImageAvailable[currentFrame];					// list of semaphores
	VkPipelineStageFlags waitStages[] = {				
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};
	submitInfo.pWaitDstStageMask = waitStages;							// stages to check semaphores at
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->vkCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;								// number of semaphores to signal
	submitInfo.pSignalSemaphores = &this->vkSemRenderFinished[currentFrame];

	VkResult result = vkQueueSubmit(this->vkGraphicsQueue, 1, &submitInfo, vkDrawFences[currentFrame]);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit Comand buffer to Graphics Queue.");
	}

	// -- 3
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &vkSemRenderFinished[currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &this->vkSwapchain;
	presentInfo.pImageIndices = &imageIndex;					// index of images in swapchain to present

	result = vkQueuePresentKHR(this->vkPresentationQueue, &presentInfo);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present image.");
	}

	// Get next frame (use % MAX_FRAME_DRAWS to keep value below MAX_FRAME_DRAWS)
	currentFrame = (currentFrame + 1) % MAX_FRAME_DRAWS;
}

void VulkanRenderer::applyLighting()
{
	for (int i = 0; i < lightSources.size(); i++)
	{

	}
}

bool VulkanRenderer::isModelInRenderer(uint32_t id)
{
	auto it = std::find_if(modelsToRender.begin(), modelsToRender.end(), 
		[id](VkModel* model) {return model->id == id;});
	return it != modelsToRender.end();;
}

bool VulkanRenderer::addToRenderer(const Model& model, glm::vec3 color)
{
	// NOT IMPLEMENTED
	return false;
}

bool VulkanRenderer::addToRendererTextured(const Model& model)
{
	// If mesh is not in renderer
	if (!isModelInRenderer(model.id))
	{
		VkUtils::VkSamplerDescriptorSetCreateInfo createInfo = {};
		createInfo.sampler = this->vkTextureSampler;
		createInfo.samplerDescriptorSetLayout = this->vkSamplerDescriptorSetLayout;

		VkModel* vkModel = new VkModel(model.id, model, context, createInfo);
		modelsToRender.push_back(vkModel);

		return true;
	}

	return false;
}

bool VulkanRenderer::updateModelTransform(int modelId, glm::mat4 newTransform)
{
	VkModel* model = getModel(modelId);
	if (model != nullptr)
	{
		model->setTransform(newTransform);
	}

	return model != nullptr;
}

void VulkanRenderer::setCamera(BaseCamera* camera)
{
	this->mainCamera = camera;
}

bool VulkanRenderer::addLightSource(Light* light)
{
	if (lightSources.size() < MAX_LIGHT_SOURCES)
	{
		lightSources.push_back(light);
		return true;
	}
	return false;
}

bool VulkanRenderer::removeFromRenderer(int modelId)
{
	auto it = std::find_if(modelsToRender.begin(), modelsToRender.end(),
		[modelId](VkModel* model) {return model->id == modelId;});
	if (it != modelsToRender.end())
	{
		VkModel* model = *it;
		modelsToDestroy.push_back(model);
		modelsToRender.erase(it);
		return true;
	}

	return false;
}

void VulkanRenderer::setupDebugMessenger()
{
	if (!ENABLE_VALIDATION_LAYERS)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	VkUtils::populateDebugMessengerCreateInfo(createInfo);

	if (VkUtils::createDebugUtilsMessengerEXT(this->vkInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to set up debug messenger.");
	}
}

/// <summary>
/// Checks whethers passed extensions are supported by Vulkan Instance.
/// </summary>
/// <param name="extensionsToCheck"></param>
/// <returns></returns>
bool VulkanRenderer::isInstanceExtensionsSupported(std::vector<const char*>* extensionsToCheck)
{
	// Getting the count of supported extensions
	uint32_t extensionsCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);

	// Getting supported extensions list using retrieved extensions count
	std::vector<VkExtensionProperties> extensions(extensionsCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.data());

	// Checking whether all extensions to check are present within supported extensions
	for (const auto& extensionToCheck : *extensionsToCheck)
	{
		bool hasExtension = false;
		for (const auto& extenstion : extensions)
		{
			if (strcmp(extensionToCheck, extenstion.extensionName))
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
		{
			return false;
		}

		return true;
	}

	return false;
}

/// <summary>
/// Checks whether device supports specified and required extensions
/// </summary>
/// <param name="device"></param>
/// <returns></returns>
bool VulkanRenderer::isDeviceSupportsRequiredExtensions(VkPhysicalDevice device)
{
	// Get device extensions count
	uint32_t extensionsCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

	// If there is no extensions, then failure
	if (extensionsCount == 0)
	{
		return false;
	}

	// Populate device extensions
	std::vector<VkExtensionProperties> extensionsProperties(extensionsCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, extensionsProperties.data());

	for (const auto& deviceExtension : deviceExtensions)
	{
		bool hasExtension = false;
		for (const auto& extension : extensionsProperties)
		{
			if (strcmp(deviceExtension, extension.extensionName) == 0)
			{
				hasExtension = true;
				break;
			}
		}

		if (!hasExtension)
		{
			return false;
		}
	}

	return true;
}

/*
	ImGui initialization, creation and cleanup functions 
*/

/// <summary>
/// Initializes IMGUI context and required IMGUI implementations
/// </summary>
void VulkanRenderer::setupImgui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	this->imguiIO = ImGui::GetIO(); (void)imguiIO;
	imguiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(this->window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.ApiVersion = VK_API_VERSION_1_4;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
	init_info.Instance = this->vkInstance;
	init_info.PhysicalDevice = this->vkPhysicalDevice;
	init_info.Device = this->vkLogicalDevice;
	init_info.QueueFamily = this->getQueueFamilies(vkPhysicalDevice).graphicsFamily;
	init_info.Queue = this->vkGraphicsQueue;
	init_info.DescriptorPool = this->imguiDescriptorPool;
	init_info.RenderPass = this->vkRenderPass;
	init_info.Subpass = 1;
	init_info.MinImageCount = 2;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.CheckVkResultFn = VkUtils::check_vk_result;
	ImGui_ImplVulkan_Init(&init_info);
}

/// <summary>
/// Prepares new imgui frame. Look for actual Imgui draw call in recordCommands() method.
/// </summary>
/// <returns></returns>
ImDrawData* VulkanRenderer::drawImgui()
{
	// Start the Dear ImGui frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	this->imguiCallback();

	// Rendering
	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();

	return draw_data;
}

/// <summary>
/// Creates descriptor pool required for IMGUI context. Descriptor sets creation is handled by IMGUI itself.
/// </summary>
void VulkanRenderer::createImguiDescriptorPool()
{
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE },
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 0;
	for (VkDescriptorPoolSize& pool_size : pool_sizes)
		pool_info.maxSets += pool_size.descriptorCount;
	pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	pool_info.pPoolSizes = pool_sizes;
	VkResult result  = vkCreateDescriptorPool(this->vkLogicalDevice, &pool_info, nullptr, &this->imguiDescriptorPool);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create IMGUI Descriptor pool.");
	}
}

void VulkanRenderer::cleanupImgui()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	vkDestroyDescriptorPool(this->vkLogicalDevice, this->imguiDescriptorPool, nullptr);
}

/*
	Vulkan helper functions
*/



bool VulkanRenderer::isDeviceSuitable(VkPhysicalDevice device)
{
	//VkPhysicalDeviceProperties deviceProperties;
	//vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	return getQueueFamilies(device).isValid()
		&& isDeviceSupportsRequiredExtensions(device)
		&& getSwapChainDetails(device).isValid()
		&& deviceFeatures.samplerAnisotropy;
}

VkFormat VulkanRenderer::defineSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags)
{
	for (VkFormat format : formats)
	{
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(this->vkPhysicalDevice, format, &properties);
		if ((tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & featureFlags) == featureFlags)
			|| (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & featureFlags) == featureFlags))
		{
			return format;
		}
	}

	throw std::runtime_error("Failed to find a matching format.");
}

VkUtils::QueueFamilyIndices VulkanRenderer::getQueueFamilies(VkPhysicalDevice device)
{
	VkUtils::QueueFamilyIndices indices;

	// Get all Queue Family Properties info for given physical device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties.data());

	// Go through each queue family and check if it has at least 1 of the required types of queue
	for (int i = 0; i < queueFamilyCount; i++)
	{
		auto queueFamily = queueFamilyProperties[i];

		// First check if queue family has at least 1 queue an this family (possibly none).
		// Queue can be multiple types defined by bitfield. We need to bitwise flags AND with VK_QUEUE_***_BIT
		// to check if it has required type.
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		// Checking if current Queue Family supports presentation (which is not a distinct queue family, can be graphics one)
		VkBool32 presentationSupport;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->vkSurface, &presentationSupport);
		if (queueFamily.queueCount > 0 && presentationSupport)
		{
			indices.presentationFamily = i;
		}


		if (indices.isValid())
		{
			break;
		}
	}

	return indices;
}



void VulkanRenderer::createTextureSampler()
{
	VkSamplerCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.magFilter = VK_FILTER_LINEAR;
	createInfo.minFilter = VK_FILTER_LINEAR;
	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.unnormalizedCoordinates = VK_FALSE;
	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.mipLodBias = 0.0f;
	createInfo.maxLod = 0.0f;
	createInfo.minLod = 0.0f;
	createInfo.anisotropyEnable = VK_FALSE;
	createInfo.maxAnisotropy = 16;

	VkResult result = vkCreateSampler(this->vkLogicalDevice, &createInfo, nullptr, &this->vkTextureSampler);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture sampler.");
	}
}

VkUtils::SwapChainDetails VulkanRenderer::getSwapChainDetails(VkPhysicalDevice device)
{
	VkUtils::SwapChainDetails details = {};

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->vkSurface, &details.surfaceCapabilities);
	
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->vkSurface, &formatCount, nullptr);
	if (formatCount != 0)
	{
		details.surfaceFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->vkSurface, &formatCount, details.surfaceFormats.data());
	}

	uint32_t presentationModesCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->vkSurface, &presentationModesCount, nullptr);
	if (presentationModesCount > 0)
	{
		details.presentationModes.resize(presentationModesCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->vkSurface, &presentationModesCount, details.presentationModes.data());
	}

	return details;
}

bool VulkanRenderer::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

VkModel* VulkanRenderer::getModel(uint32_t id)
{
	auto it = std::find_if(modelsToRender.begin(), modelsToRender.end(),
		[id](VkModel* model) {return model->id == id;});
	return it != modelsToRender.end() ? *it : nullptr;
}

void VulkanRenderer::printPhysicalDeviceInfo(VkPhysicalDevice device, bool printPropertiesFull, bool printFeaturesFull)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	printf("%Device ID %d\n", deviceProperties.deviceID);
	printf("%s (%d)\n", deviceProperties.deviceName, deviceProperties.deviceType);
	printf("API version: %d\n", deviceProperties.apiVersion);
	printf("Driver version %d\n", deviceProperties.driverVersion);
	printf("Vendor ID %d\n", deviceProperties.vendorID);

	if (printPropertiesFull)
	{
		// TODO Print full properties
	}

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	
	// TODO Print device features info
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = code.size();
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(this->vkLogicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module.");
	}

	return shaderModule;
}
