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

	shader->setUniform("lightColor", light->color);
	shader->setUniform("lightPos", light->position);
	shader->setUniform("ambientStrength", light->ambientStrength);
	shader->setUniform("specularStrength", light->specularStrength);
	shader->setUniform("shininess", light->shininess);

	shader->setUniform("viewPos", this->camera->getPosition());

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

void OpenGLRenderer::setLight(LightSource* lightSource)
{
	this->light = lightSource;
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
