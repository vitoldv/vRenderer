#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <set>
#include <array>
#include <algorithm>
#include <map>
#include <functional>

#include "display_settings.h"
#include "IRenderer.h"
#include "Lighting.h"
#include "Model.h"
#include "VkModel.h"
#include "VulkanUtils.h"
#include "BaseCamera.h"
#include "Lighting.h"

#ifdef NDEBUG
#define ENABLE_VALIDATION_LAYERS false
#else
#define ENABLE_VALIDATION_LAYERS true
#endif

// preferrable surface settings (selected if supported)
#define SURFACE_COLOR_FORMAT		VK_FORMAT_R8G8B8A8_UNORM
#define SURFACE_COLOR_SPACE			VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
#define SURFACE_PRESENTATION_MODE	VK_PRESENT_MODE_MAILBOX_KHR

#define BACKGROUND_COLOR 0x008B8BFF

#define MAX_FRAME_DRAWS 2
#define MAX_LIGHT_SOURCES 10

// Names of extensions required to run the application
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_EXT_ROBUSTNESS_2_EXTENSION_NAME
};

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

class VulkanRenderer : public IRenderer
{
private:
	GLFWwindow* window;

	int currentFrame = 0;

	VkUtils::VkContext context;

	// Native Vulkan Components
	VkInstance vkInstance; 
	VkPhysicalDevice vkPhysicalDevice;
	VkDevice vkLogicalDevice;
	VkQueue vkGraphicsQueue;
	VkQueue vkPresentationQueue;
	VkSurfaceKHR vkSurface;
	VkSwapchainKHR vkSwapchain;
	std::vector<VkUtils::SwapChainImage> swapchainImages;

	// Graphics pipeline
	VkRenderPass vkRenderPass;
	VkPipeline vkGraphicsPipeline;
	VkPipelineLayout vkPipelineLayout;
	VkPipeline vkSecondPipeline;
	VkPipelineLayout vkSecondPipelineLayout;

	std::vector<VkFramebuffer> vkSwapchainFramebuffers;
	VkCommandPool vkGraphicsCommandPool;
	std::vector<VkCommandBuffer> vkCommandBuffers;

	std::vector<VkImage> colorBufferImage;
	std::vector<VkDeviceMemory> colorBufferImageMemory;
	std::vector<VkImageView> colorBufferImageView;
	VkFormat colorFormat;

	std::vector<VkImage> depthBufferImage;
	std::vector<VkDeviceMemory> depthBufferImageMemory;
	std::vector<VkImageView> depthBufferImageView;
	VkFormat depthFormat;

	// Extension Vulkan Components
	VkDebugUtilsMessengerEXT debugMessenger;

	// Descriptors
	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkDescriptorSetLayout vkSamplerDescriptorSetLayout;
	VkDescriptorSetLayout vkInputDescriptorSetLayout;		// input to subpass 2

	VkDescriptorPool vkDescriptorPool;
	VkDescriptorPool vkInputDescriptorPool;
	std::vector<VkDescriptorSet> vkDescriptorSets;
	std::vector<VkDescriptorSet> vkInputDescriptorSets;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	VkDeviceSize minUniformBufferOffset;
	VkPushConstantRange vkPushConstantRange;

	// LEFT FOR REFERENCE ON DYNAMIC UNIFORM BUFFERS
	// UboModel* modelTransferSpace;	
	// size_t modelUniformAlignment;
	//std::vector<VkBuffer> uniformBuffersDynamic;
	//std::vector<VkDeviceMemory> uniformBuffersMemoryDynamic;

	// Utility
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkSemaphore> vkSemImageAvailable;
	std::vector<VkSemaphore> vkSemRenderFinished;
	std::vector<VkFence> vkDrawFences;

	// Scene
	BaseCamera* mainCamera;
	std::vector<VkModel*> modelsToRender;
	std::vector<VkModel*> modelsToDestroy;

	std::vector<Light*> lightSources;
	struct UboLightArray {
		UboLight lights[10];       // Total size: 96 * 10 = 960 bytes
	} uboLightArray;
	
	std::vector<VkBuffer> lightUniformBuffers;
	std::vector<VkDeviceMemory> lightUniformMemory;
	VkDescriptorSetLayout vkLightDescriptorSetLayout;
	std::vector<VkDescriptorSet> vkLightDescriptorSets;

	// Textures
	VkSampler vkTextureSampler;

	/*
	---- IMGUI fields -----
	*/
	ImGuiIO imguiIO;
	VkDescriptorPool imguiDescriptorPool;
	std::function<void()> imguiCallback;

public:
	VulkanRenderer();

	int init(GLFWwindow* window);
	void draw();
	void applyLighting();

	bool addToRenderer(const Model& model, glm::vec3 color);
	bool addToRendererTextured(const Model& model);
	bool removeFromRenderer(int modelId);
	bool isModelInRenderer(uint32_t id);

	bool updateModelTransform(int modelId, glm::mat4 newTransform);
	void setCamera(BaseCamera* camera);
	bool addLightSource(Light* light);

	void cleanup();

	void setImguiCallback(std::function<void()> callback);

	~VulkanRenderer();

private:
	
	void createVulkanInstance();
	void retrievePhysicalDevice();
	void createLogicalDevice();
	void createSurface();
	void createSwapChain();
	void createRenderPass();
	void createGraphicsPipeline();
	void createColorBufferImage();
	void createDepthBuffer();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSyncTools();
	void createDescriptorSetLayout();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	void createInputDescriptorSets();
	void createPushConstantRange();
	void createTextureSampler();

	/* 
	---- IMGUI functions -----
	*/
	void setupImgui();
	void createImguiDescriptorPool();
	ImDrawData* drawImgui();
	void cleanupImgui();

	void setupDebugMessenger();

	VkSurfaceFormatKHR defineSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR definePresentationMode(const std::vector<VkPresentModeKHR> presentationModes);
	VkExtent2D defineSwapChainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);
	VkFormat defineSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);

	VkShaderModule createShaderModule(const std::vector<char>& code);
	void recordCommands(uint32_t currentImage, ImDrawData& imguiDrawData);
	void updateUniformBuffers(uint32_t imageIndex);
	
	// LEFT FOR REFERENCE ON DYNAMIC UNIFORM BUFFERS
	//void allocateDynamicBufferTransferSpace();

	bool isInstanceExtensionsSupported(std::vector<const char*>* extensions);
	bool isDeviceSupportsRequiredExtensions(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);

	VkUtils::QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
	VkUtils::SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);
	bool checkValidationLayerSupport();

	VkModel* getModel(uint32_t id);

	void printPhysicalDeviceInfo(VkPhysicalDevice device, bool printPropertiesFull = false, bool printFeaturesFull = false);
};


// ------------------- TODO -------------------
// 1. Make a generic function for pipeline creation to avoid a lot of duplicated code in createGraphicsPipeline();
// 2. Create a solution for removing swapChainImages.size() overuse (Create a constant value for images count or 
// make a wrapper for single swapchain image code);
// 3. Think of single memory buffer for meshes related to specific model
// 4. Create solution for instance batching
// 5. Extract all duplicated "magic" Vulkan flags to single place in code
// 6. Add mipmap generation for textures