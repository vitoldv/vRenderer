#pragma once

#include <glad/glad.h>

#include <stdexcept>
#include <string>

#include "Texture.h"

class GLTexture
{
public:

	const std::string name;
	uint32_t glId;

	GLTexture(const Texture& texture);
	~GLTexture();

private:

	void createTexture(const Texture& texture);
	void cleanup();
};