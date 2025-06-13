#pragma once

#include <string>
#include <cstdint>

struct Texture
{
	~Texture()
	{
		free(ptr);
	}

	uint8_t* ptr = nullptr;
	uint32_t height = 0;			// pixels
	uint32_t width = 0;			// pixels
	uint32_t size = 0;			// byte size
	std::string name;
};