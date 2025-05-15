#include "OpenGLRenderer.h"

uint32_t shaderProgram;
GLMesh* mesh;
GLTexture* texture;

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
	{
		int  success;
		char infoLog[512];

		// Compile vertex shader
		unsigned int vertexShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		std::string str = GLUtils::readFile("vRenderer\\shaders\\opengl\\shader.vert").c_str();
		const char* vertexShaderSource = str.c_str();
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// Compile fragment shader
		unsigned int fragmentShader;
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		str = GLUtils::readFile("vRenderer\\shaders\\opengl\\shader.frag").c_str();
		const char* fragmentShaderSource = str.c_str();
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// Create shader program
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER PROGRAM COMPILATION FAILED\n" << infoLog << std::endl;
		}

		// Deleting shaders
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

	}

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
	glUseProgram(shaderProgram);
	
	// Setting uniforms
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(this->camera->getViewMatrix()));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(this->camera->getProjectionMatrix()));

	for (auto* model : modelsToRender)
	{
		model->draw(shaderProgram);
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
