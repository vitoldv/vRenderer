#pragma once

#include <glad/glad.h>

#include "IRenderer.h"

class OpenGLRenderer : public IRenderer
{
	// Inherited via IRenderer
	int init(GLFWwindow* window) override;
	void draw() override;
	bool addToRenderer(const Model& model, glm::vec3 color) override;
	bool addToRendererTextured(const Model& model) override;
	bool removeFromRenderer(int modelId) override;
	bool isModelInRenderer(uint32_t id) override;
	bool updateModelTransform(int modelId, glm::mat4 newTransform) override;
	void setCamera(BaseCamera* camera) override;
	void cleanup() override;
	void setImguiCallback(std::function<void()> callback) override;
};