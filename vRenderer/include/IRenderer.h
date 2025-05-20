#pragma once

#include <GLFW/glfw3.h>
#include <functional>

#include "Lighting.h"
#include "Model.h"
#include "BaseCamera.h"

/// <summary>
/// Interface for a 3D renderer class
/// </summary>
class IRenderer
{
public:
	virtual int init(GLFWwindow* window) = 0;
	virtual void draw() = 0;

	virtual bool addToRenderer(const Model& model, glm::vec3 color) = 0;
	virtual bool addToRendererTextured(const Model& model) = 0;
	virtual bool removeFromRenderer(int modelId) = 0;
	virtual bool isModelInRenderer(uint32_t id) = 0;

	virtual bool updateModelTransform(int modelId, glm::mat4 newTransform) = 0;
	virtual void setCamera(BaseCamera* camera) = 0;
	virtual void setLight(LightSource* lightSource) = 0;

	virtual void cleanup() = 0;

	virtual void setImguiCallback(std::function<void()> callback) = 0;
};