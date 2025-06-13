#include "GLTexture.h"

GLTexture::GLTexture(const Texture& texture) :
	name(texture.name)
{
	createTexture(texture);
}

GLTexture::~GLTexture()
{
	cleanup();
}

void GLTexture::createTexture(const Texture& texture)
{
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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.width, texture.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.ptr);
	glGenerateMipmap(GL_TEXTURE_2D);

	// I nearly ate my brain out in attempts to grasp why the fuck my shader was sampling stuff I 'never' bind...
	glBindTexture(GL_TEXTURE_2D, 0);
}

void GLTexture::cleanup()
{
	glDeleteTextures(1, &glId);
	glBindTexture(GL_TEXTURE_2D, 0);
}
