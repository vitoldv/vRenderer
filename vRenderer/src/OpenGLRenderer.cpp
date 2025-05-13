#include "OpenGLRenderer.h"

int OpenGLRenderer::init(GLFWwindow* window)
{
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);

	return 0;
}

void OpenGLRenderer::draw()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

bool OpenGLRenderer::addToRenderer(const Model& model, glm::vec3 color)
{
	return false;
}

bool OpenGLRenderer::addToRendererTextured(const Model& model)
{
	return false;
}

bool OpenGLRenderer::removeFromRenderer(int modelId)
{
	return false;
}

bool OpenGLRenderer::isModelInRenderer(uint32_t id)
{
	return false;
}

bool OpenGLRenderer::updateModelTransform(int modelId, glm::mat4 newTransform)
{
	return false;
}

void OpenGLRenderer::setCamera(BaseCamera* camera)
{
}

void OpenGLRenderer::cleanup()
{
}

void OpenGLRenderer::setImguiCallback(std::function<void()> callback)
{
}
