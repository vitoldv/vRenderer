#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <iostream>

namespace GLUtils
{
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec3 normal;
		glm::vec2 uv;
	};

	static std::string readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			throw std::runtime_error("Failed to open file.");
		}

		size_t fileSize = (size_t)file.tellg();

		file.seekg(0);

		std::string str;
		str.resize(fileSize);
		file.read(str.data(), fileSize);

		file.close();

		return str;
	}

	static std::array<float, 4> getRGBANormalized(uint32_t color) 
	{
		return 
		{
			((color >> 24) & 0xFF) / 255.0f,  // R
			((color >> 16) & 0xFF) / 255.0f,  // G
			((color >> 8) & 0xFF) / 255.0f,   // B
			(color & 0xFF) / 255.0f           // A
		};
	}
}