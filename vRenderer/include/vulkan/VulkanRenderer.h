#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#ifdef NDEBUG
#define ENABLE_VALIDATION_LAYERS false
#define ASSERTIONS_ENABLED 0
#else
#define ENABLE_VALIDATION_LAYERS true
#define ASSERTIONS_ENABLED 1
#endif

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
#include "error_handling.h"
#include "IRenderer.h"
#include "Lighting.h"
#include "ModelInstance.h"
#include "VkModel.h"
#include "VkSimpleMesh.h"
#include "VulkanUtils.h"
#include "VkUniform.hpp"
#include "VkUniformDynamic.hpp"
#include "VkImageWrapper.h"
#include "VkSetLayoutFactory.h"
#include "BaseCamera.h"
#include "VkSkyboxPipeline.h"
#include "VkShaderManager.h"
#include "VkCubemap.h"
#include "VkSkybox.h"
#include "VkOutlinePipeline.h"
#include "VkMainPipeline.h"
#include "VkSecondPassPipeline.h"

// preferrable surface settings (selected if supported)
#define SURFACE_COLOR_FORMAT		VK_FORMAT_R8G8B8A8_UNORM
#define SURFACE_COLOR_SPACE			VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
#define SURFACE_PRESENTATION_MODE	VK_PRESENT_MODE_MAILBOX_KHR

#define BACKGROUND_COLOR 0x008B8BFF

#define IMAGE_COUNT 3			// the number of images in swapchain
#define MAX_FRAME_DRAWS 2

// Names of extensions required to run the application
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_EXT_ROBUSTNESS_2_EXTENSION_NAME
};

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

using namespace VRD::Scene;

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

	VkSamplerDescriptorSetCreateInfo samplerDescriptorCreateInfo;

	// Graphics pipeline
	VkRenderPass renderPass;
	std::unique_ptr<VkMainPipeline> mainPipeline;
	std::unique_ptr<VkSecondPassPipeline> secondPassPipeline;

	std::unique_ptr<VkOutlinePipeline> outlinePipeline;

	std::vector<VkFramebuffer> swapchainFramebuffers;
	VkCommandPool graphicsCommandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	// Extension Vulkan Components
	VkDebugUtilsMessengerEXT debugMessenger;

	std::unique_ptr<VkImageWrapper> depthImage;
	std::unique_ptr<VkImageWrapper> colorImage;

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

	std::shared_ptr<RenderSettings> renderSettings;
	// Scene
	std::shared_ptr<BaseCamera> sceneCamera;
	std::vector<VkModel*> modelsToRender;
	std::vector<VkModel*> modelsToDestroy;
	std::vector<std::shared_ptr<Light>> lightSources;
	UboLightArray uboLightArray;

	// Uniforms
	std::unique_ptr<VkUniform<UboLightArray>> lightUniform;
	std::unique_ptr<VkUniform<UboViewProjection>> vpUniform;
	std::unique_ptr<VkUniformDynamic<UboDynamicColor>> colorUniformsDynamic;
	std::unique_ptr<VkUniform<UboPostProcessingFeatures>> postPrFeaturesUniform;

	// Textures
	VkSampler textureSampler;

	bool renderSkybox;
	std::unique_ptr<VkSkybox> skybox;
	std::unique_ptr<VkSkyboxPipeline> skyboxPipeline;

	/*
	---- IMGUI fields -----
	*/
	ImGuiIO imguiIO;
	VkDescriptorPool imguiDescriptorPool;
	std::function<void()> imguiCallback;

public:

	~VulkanRenderer();

	int init(GLFWwindow* window);
	void draw();
	void applyLighting();

	bool addToRenderer(const Model& model, glm::vec3 color);
	bool addToRendererTextured(const ModelInstance& model);
	bool removeFromRenderer(int modelId);
	bool isModelInRenderer(uint32_t id);

	bool updateModelTransform(int modelId, glm::mat4 newTransform);
	void setCamera(const std::shared_ptr<BaseCamera> camera);
	bool addLightSources(const std::shared_ptr<Light> light[], uint32_t count);
	bool removeLightSources(uint32_t* ids, uint32_t count) override;
	void bindRenderSettings(const std::shared_ptr<RenderSettings> renderSettings);

	void setImguiCallback(std::function<void()> callback);

	void cleanup();

private:
	
	void createVulkanInstance();
	void createSurface();
	void retrievePhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createDepthBuffer();
	void createColorBufferImage();
	void createRenderPass();
	
	void createDescriptorSetLayouts();
	void createTextureSampler();
	void createPushConstantRange();
	void createDescriptorPools();
	void createUniforms();
	void createSubpassInputDescriptorSets();
	void createGraphicsPipeline();
	
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void createSyncTools();

	void recordCommands(uint32_t currentImage, ImDrawData& imguiDrawData);
	void updateUniforms(uint32_t imageIndex);

	void setupDebugMessenger();

	VkSurfaceFormatKHR defineSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
	VkPresentModeKHR definePresentationMode(const std::vector<VkPresentModeKHR> presentationModes);
	VkExtent2D defineSwapChainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);
	VkFormat defineSupportedFormat(const std::vector<VkFormat>& formats, VkImageTiling tiling, VkFormatFeatureFlags featureFlags);

	bool isInstanceExtensionsSupported(std::vector<const char*>* extensions);
	bool isDeviceSupportsRequiredExtensions(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);

	QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
	SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);
	bool checkValidationLayerSupport();

	VkModel* getModel(uint32_t id);

	void printPhysicalDeviceInfo(VkPhysicalDevice device, bool printPropertiesFull = false, bool printFeaturesFull = false);

	/*
	---- IMGUI functions -----
	*/
	void setupImgui();
	void createImguiDescriptorPool();
	ImDrawData* drawImgui();
	void cleanupImgui();

	bool setSkybox(const std::shared_ptr<Cubemap> cubemap) override;
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