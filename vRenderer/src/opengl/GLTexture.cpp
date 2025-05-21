#include "GLTexture.h"
#include "stb_image.h"

GLTexture::GLTexture(std::string fileName) : 
	name(fileName.c_str())
{
	createTexture(fileName);
}

GLTexture::~GLTexture()
{
	cleanup();
}


stbi_uc* loadTexture(std::string fileName, int& width, int& height)
{
	int channels;
	stbi_uc* image = stbi_load(fileName.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!image)
	{
		throw std::runtime_error("Failed to load texture \"" + fileName + "\".");
	}

	return image;
}

void GLTexture::createTexture(std::string fileName)
{
	int width, height;
	stbi_uc* data = loadTexture(fileName, width, height);

	glGenTextures(1, &glId);
	glBindTexture(GL_TEXTURE_2D, glId);

	// Sampler settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Anisotropic filtering
	GLfloat maxAnisotropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	// I nearly ate my brain out in attempts to grasp why the fuck my shader was sampling stuff I 'never' bind...
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture::cleanup()
{
	glDeleteTextures(1, &glId);
	glBindTexture(GL_TEXTURE_2D, 0);
}
