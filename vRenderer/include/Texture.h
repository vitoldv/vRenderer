#pragma once

#include <string>
#include <array>
#include <memory>
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


struct Cubemap
{
	std::shared_ptr<Texture> back;
	std::shared_ptr<Texture> bottom;
	std::shared_ptr<Texture> front;
	std::shared_ptr<Texture> left;
	std::shared_ptr<Texture> right;
	std::shared_ptr<Texture> top;

	uint32_t getTotalSize() const
	{
		return back->size + bottom->size + front->size + left->size + right->size + top->size;
	}

	void copyTextureData(void* dst) const
	{
		uint8_t* base = static_cast<uint8_t*>(dst);
		size_t offset = 0;
		memcpy(static_cast<void*>(base + offset), right->ptr, right->size);
		offset += right->size;
		memcpy(static_cast<void*>(base + offset), left->ptr, left->size);
		offset += left->size;
		memcpy(static_cast<void*>(base + offset), top->ptr, top->size);
		offset += top->size;
		memcpy(static_cast<void*>(base + offset), bottom->ptr, bottom->size);
		offset += bottom->size;
		memcpy(static_cast<void*>(base + offset), front->ptr, front->size);
		offset += front->size;
		memcpy(static_cast<void*>(base + offset), back->ptr, back->size);
	}

	void getFaceExtent(uint32_t& outWidth, uint32_t& outHeight) const
	{
		outWidth = back->width;
		outHeight = back->height;
	}
};