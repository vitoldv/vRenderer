#pragma once

#include <stdint.h>

struct RenderSettings
{
	enum API
	{
		VULKAN,
		OPENGL
	};

	API api;
	uint32_t backgroundColor;
};
