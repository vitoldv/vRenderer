#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <memory>

#include "RenderSettings.h"
#include "Lighting.h"
#include "ModelInstance.h"
#include "BaseCamera.h"

using namespace VRD::Scene;

/// <summary>
/// Interface for a 3D renderer class
/// </summary>
class IRenderer
{
public:
	virtual ~IRenderer() = default;

	virtual int init(GLFWwindow* window) = 0;
	virtual void setImguiCallback(std::function<void()> callback) = 0;

	virtual void draw() = 0;
	virtual bool updateModelTransform(int modelId, glm::mat4 newTransform) = 0;

	virtual bool addToRenderer(const Model& model, glm::vec3 color) = 0;
	virtual bool addToRendererTextured(const ModelInstance& model) = 0;
	virtual bool removeFromRenderer(int modelId) = 0;
	virtual bool isModelInRenderer(uint32_t id) = 0;

	virtual void bindRenderSettings(const std::shared_ptr<RenderSettings> renderSettings) = 0;
	virtual void setCamera(const std::shared_ptr<BaseCamera> camera) = 0;
	virtual bool addLightSources(const std::shared_ptr<Light> lights[], uint32_t count) = 0;

	virtual void cleanup() = 0;
};