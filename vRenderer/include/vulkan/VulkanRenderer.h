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
#include "VkUniform.hpp"
#include "VkUniformDynamic.hpp"
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

#define IMAGE_COUNT 3			// the number of images in swapchain
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
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	std::vector<VkUtils::SwapChainImage> swapchainImages;

	std::array<VkDescriptorSetLayout, 4> setLayoutMap;
	VkSamplerDescriptorSetCreateInfo samplerDescriptorCreateInfo;


	// Graphics pipeline
	VkRenderPass renderPass;
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;
	VkPipeline secondPipeline;
	VkPipelineLayout secondPipelineLayout;

	std::vector<VkFramebuffer> swapchainFramebuffers;
	VkCommandPool graphicsCommandPool;
	std::vector<VkCommandBuffer> commandBuffers;

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
	VkDescriptorSetLayout inputDescriptorSetLayout;		// input to subpass 2

	VkDescriptorPool uniformDescriptorPool;
	VkDescriptorPool dynamicUniformDescriptorPool;
	VkDescriptorPool inputDescriptorPool;
	std::vector<VkDescriptorSet> inputDescriptorSets;

	VkPushConstantRange pushConstantRange;

	// Utility
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkSemaphore> semImageAvailable;
	std::vector<VkSemaphore> semRenderFinished;
	std::vector<VkFence> drawFences;

	// Scene
	std::shared_ptr<BaseCamera> sceneCamera;
	std::vector<VkModel*> modelsToRender;
	std::vector<VkModel*> modelsToDestroy;

	std::vector<std::shared_ptr<Light>> lightSources;
	struct UboLightArray {
		UboLight lights[MAX_LIGHT_SOURCES];       // Total size: 96 * 10 = 960 bytes
	} uboLightArray;

	// Uniforms
	std::vector<std::unique_ptr<VkUniform<UboLightArray>>> lightUniforms;
	std::vector<std::unique_ptr<VkUniform<UboViewProjection>>> vpUniforms;
		
	std::vector<std::unique_ptr<VkUniformDynamic<UboDynamicColor>>> colorUniformsDynamic;
	// Textures
	VkSampler textureSampler;

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
	void setCamera(const std::shared_ptr<BaseCamera> camera);
	bool addLightSources(const std::shared_ptr<Light> light[], uint32_t count);

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
	void createDescriptorSetLayouts();
	void createUniforms();
	void createDescriptorPools();
	void createSubpassInputDescriptorSets();
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

	bool isInstanceExtensionsSupported(std::vector<const char*>* extensions);
	bool isDeviceSupportsRequiredExtensions(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);

	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
	SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);
	bool checkValidationLayerSupport();

	VkModel* getModel(uint32_t id);

	void printPhysicalDeviceInfo(VkPhysicalDevice device, bool printPropertiesFull = false, bool printFeaturesFull = false);
};


// ------------------- TODO -------------------
// 3. Think of single memory buffer for meshes related to specific model
// 4. Create solution for instance batching
// 5. Extract all duplicated "magic" Vulkan flags to single place in code
// 6. Add mipmap generation for textures

// 7. Resolve dynamic uniform issue.
// Details: currently dynamic uniforms have no useful utilization. But they actually might be very helpful.
// The main question still unresolved in implementation is a "scope" of a dynamic uniform.
// As a "scope" I mean an abstract geometry unit which has unique dynamic uniform value. It might be the smallest "mesh"
// as well as model that holds it. Current dummy implementation works per-model i.e. holds one single value.
// But I tested it, and per-mesh approach works too, it just requires proper MAX_OBJECTS value (which could be extracted to a variable)
// and per-mesh cmdBind call, which can be achieved either throught lambda callback in model->draw or the model itself could hold the uniform
// (which is less preferrable due to pipeline data dependecy like currentImage. Also I hadn't tested it).