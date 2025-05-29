#include "GLShader.h"

GLShader::GLShader(const char* vertShaderPath, const char* fragShaderPath) :
	vertShaderPath(vertShaderPath),
	fragShaderPath(fragShaderPath)
{
	compileShader();
}

void GLShader::enable()
{
	glUseProgram(glId);
}

void GLShader::cleanup()
{
}

void GLShader::compileShader()
{
	int  success;
	char infoLog[512];

	// Compile vertex shader
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	std::string str = GLUtils::readFile(vertShaderPath).c_str();
	const char* vertexShaderSource = str.c_str();
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "VERTEX SHADER COMPILATION FAILED\n" << infoLog << std::endl;
	}

	// Compile fragment shader
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	str = GLUtils::readFile(fragShaderPath).c_str();
	const char* fragmentShaderSource = str.c_str();
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "FRAGMENT SHADER COMPILATION FAILED\n" << infoLog << std::endl;
	}

	// Create shader program
	this->glId = glCreateProgram();
	glAttachShader(this->glId, vertexShader);
	glAttachShader(this->glId, fragmentShader);
	glLinkProgram(this->glId);
	glGetProgramiv(this->glId, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(this->glId, 512, NULL, infoLog);
		std::cout << "SHADER PROGRAM COMPILATION FAILED\n" << infoLog << std::endl;
	}

	// Deleting shaders (no need of them now)
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

