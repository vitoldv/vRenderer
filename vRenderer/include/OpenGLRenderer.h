#pragma once

#include <functional>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "IRenderer.h"
#include "Lighting.h"
#include "GLUtils.h"
#include "GLShader.h"
#include "GLModel.h"
#include "GLTexture.h"
#include "BaseCamera.h"

#define BACKGROUND_COLOR 0x888800FF

#define MAX_LIGHT_SOURCES 10

class OpenGLRenderer : public IRenderer
{
public:

	~OpenGLRenderer();

	int init(GLFWwindow* window) override;
	void draw() override;
	bool addToRenderer(const Model& model, glm::vec3 color) override;
	bool addToRendererTextured(const Model& model) override;
	bool removeFromRenderer(int modelId) override;
	bool isModelInRenderer(uint32_t id) override;
	bool updateModelTransform(int modelId, glm::mat4 newTransform) override;
	void setCamera(const std::shared_ptr<BaseCamera> camera) override;
	bool addLightSources(const std::shared_ptr<Light> lights[], uint32_t count) override;
	void cleanup() override;
	void setImguiCallback(std::function<void()> callback) override;

private:

	GLShader* shader;

	std::shared_ptr<BaseCamera> camera;
	std::vector<GLModel*> modelsToRender;
	std::vector<std::shared_ptr<Light>> lightSources;

	GLModel* getModel(uint32_t id);

	void applyLighting();

	/*
	---- IMGUI fields -----
	*/
	ImGuiIO imguiIO;
	std::function<void()> imguiCallback;
};