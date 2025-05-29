#pragma once

#include <glad/glad.h>

#include <stdexcept>
#include <string>

class GLTexture
{
public:

	const std::string name;
	uint32_t glId;

	GLTexture(std::string fileName);
	~GLTexture();

private:

	void createTexture(std::string fileName);
	void cleanup();
};