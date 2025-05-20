#pragma once

#include <stdint.h>
#include <glad/glad.h>
#include <string>

#include "GLUtils.h"

class GLShader
{
public:
	const char* fragShaderPath;
	const char* vertShaderPath;

	uint32_t glId;

	GLShader(const char* vertShaderPath, const char* fragShaderPath);

	void enable();

	template<typename T>
	bool setUniform(const char* name, T value);

	void cleanup();

private:

	void compileShader();
};

template<typename T>
bool GLShader::setUniform(const char* name, T value)
{
	int uniformLocation = glGetUniformLocation(glId, name);

	if (uniformLocation == -1)
	{
		return false;
	}

	if constexpr (std::is_same<T, int>::value)
	{
		glUniform1i(uniformLocation, value);
	}
	else if constexpr (std::is_same<T, float>::value)
	{
		glUniform1f(uniformLocation, value);
	}
	else if constexpr (std::is_same<T, bool>::value)
	{
		glUniform1i(uniformLocation, static_cast<int>(value));
	}
	else if constexpr (std::is_same<T, glm::mat3>::value)
	{
		glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
	}
	else if constexpr (std::is_same<T, glm::mat4>::value)
	{
		glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(value));
	}
	else if constexpr (std::is_same<T, glm::vec2>::value)
	{
		glUniform2f(uniformLocation, value.x, value.y);
	}
	else if constexpr (std::is_same<T, glm::vec3>::value)
	{
		glUniform3f(uniformLocation, value.x, value.y, value.z);
	}
	else
	{
		static_assert(false, "Unsupported uniform type");
		return false;
	}

	return true;
}