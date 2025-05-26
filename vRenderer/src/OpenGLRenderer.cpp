#include "OpenGLRenderer.h"

int OpenGLRenderer::init(GLFWwindow* window)
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);

	// IMGUI
	{
		// Setup ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		this->imguiIO = ImGui::GetIO(); (void)imguiIO;
		imguiIO.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 460");
	}

	// SHADERS
	shader = new GLShader("vRenderer\\shaders\\opengl\\shader.vert", "vRenderer\\shaders\\opengl\\shader.frag");

	return 0;
}

void OpenGLRenderer::draw()
{
	// IMGUI rendering
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		this->imguiCallback();
		// Rendering
		ImGui::Render();
	}

	auto clearColor = GLUtils::getRGBANormalized(BACKGROUND_COLOR);
	glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Attach shader program
	shader->enable();
	
	// Setting uniforms
	shader->setUniform("view", this->camera->getViewMatrix());
	shader->setUniform("projection", this->camera->getProjectionMatrix());

	applyLighting();

	for (auto* model : modelsToRender)
	{
		model->draw(*shader, *camera);
	}

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool OpenGLRenderer::addToRenderer(const Model& model, glm::vec3 color)
{
	// NOT IMPLEMENTED
	return false;
}

bool OpenGLRenderer::addToRendererTextured(const Model& model)
{
	// If mesh is not in renderer
	if (!isModelInRenderer(model.id))
	{
		GLModel* glModel = new GLModel(model.id, model);
		modelsToRender.push_back(glModel);
		return true;
	}

	return false;
}

bool OpenGLRenderer::removeFromRenderer(int modelId)
{
	auto it = std::find_if(modelsToRender.begin(), modelsToRender.end(),
		[modelId](GLModel* model) {return model->id == modelId;});
	if (it != modelsToRender.end())
	{
		GLModel* model = *it;
		delete model;
		model = nullptr;
		modelsToRender.erase(it);
		return true;
	}

	return false;
}

bool OpenGLRenderer::isModelInRenderer(uint32_t id)
{
	auto it = std::find_if(modelsToRender.begin(), modelsToRender.end(),
		[id](GLModel* model) {return model->id == id;});
	return it != modelsToRender.end();;
}

GLModel* OpenGLRenderer::getModel(uint32_t id)
{
	auto it = std::find_if(modelsToRender.begin(), modelsToRender.end(),
		[id](GLModel* model) {return model->id == id;});
	return it != modelsToRender.end() ? *it : nullptr;
}

bool OpenGLRenderer::updateModelTransform(int modelId, glm::mat4 newTransform)
{
	GLModel* model = getModel(modelId);
	if (model != nullptr)
	{
		model->setTransform(newTransform);
	}

	return model != nullptr;
}

void OpenGLRenderer::setCamera(BaseCamera* camera)
{
	this->camera = camera;
}

bool OpenGLRenderer::addLightSource(Light* light)
{
	if (lightSources.size() < MAX_LIGHT_SOURCES)
	{
		lightSources.push_back(light);
		return true;
	}
	return false;
}

void OpenGLRenderer::cleanup()
{
	for (auto* model : modelsToRender)
	{
		delete model;
		model = nullptr;
	}
}

void OpenGLRenderer::setImguiCallback(std::function<void()> callback)
{
	this->imguiCallback = callback;
}

void OpenGLRenderer::applyLighting()
{
	for (int i = 0; i < lightSources.size(); i++)
	{	
		auto& light = *lightSources[i];
		std::string uniformBase = "lightSources[" + std::to_string(i) + "].";

		shader->setUniform((uniformBase + "type").c_str(), static_cast<int>(light.type));
		shader->setUniform((uniformBase + "color").c_str(), light.color);
		shader->setUniform((uniformBase + "ambientStrength").c_str(), light.ambientStrength);
		shader->setUniform((uniformBase + "specularStrength").c_str(), light.specularStrength);
		shader->setUniform((uniformBase + "shininess").c_str(), light.shininess);
		shader->setUniform((uniformBase + "constant").c_str(), light.constant);
		shader->setUniform((uniformBase + "linear").c_str(), light.linear);
		shader->setUniform((uniformBase + "quadratic").c_str(), light.quadratic);

		if (light.type == Light::Type::DIRECTIONAL || light.type == Light::Type::SPOT)
		{
			shader->setUniform(("lightDir[" + std::to_string(i) + "]").c_str(), light.direction);
		}
		if (light.type == Light::Type::POINT || light.type == Light::Type::SPOT)
		{
			// NOTE: light's position is set separately in vertex stage to be transformed into view space
			shader->setUniform(("lightPos[" + std::to_string(i) + "]").c_str(), light.position);
		}
		if (light.type == Light::Type::SPOT)
		{
			shader->setUniform((uniformBase + "cutOff").c_str(), glm::cos(glm::radians(light.cutOff)));
			shader->setUniform((uniformBase + "outerCutOff").c_str(), glm::cos(glm::radians(light.outerCutOff)));
		}
	}
}
