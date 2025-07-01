#pragma once

#include <string>
#include <array>
#include <memory>
#include <cstdint>

struct Texture
{
	Texture() {}
	~Texture()
	{
		free(ptr);
	}

	uint8_t* ptr = nullptr;
	uint32_t height = 0;		// pixel count
	uint32_t width = 0;			// pixel count
	uint32_t size = 0;			// byte size
	std::string name;

	// Copy constructor and operators are deleted for design reasons.
	// We generally do not want any duplicates of a single asset in memory.
	// So any object that wants texture's data should reference to the once imported instance of it.
	Texture(const Texture& other) = delete;
	Texture& operator=(const Texture& other) = delete;

	Texture& operator=(Texture&& other) noexcept
	{
		if (this == &other) return *this;
		
		this->ptr = other.ptr;
		this->height = other.height;
		this->width = other.width;
		this->size = other.size;
		this->name = std::move(other.name);

		other.ptr = nullptr;
		other.height = 0;
		other.width = 0;
		other.size = 0;

		return *this;
	}
};


struct Cubemap
{
	Cubemap() {}

	Texture back;
	Texture bottom;
	Texture front;
	Texture left;
	Texture right;
	Texture top;

	uint32_t getTotalSize() const
	{
		return back.size + bottom.size + front.size + left.size + right.size + top.size;
	}

	void copyTextureData(void* dst) const
	{
		uint8_t* base = static_cast<uint8_t*>(dst);
		size_t offset = 0;
		memcpy(static_cast<void*>(base + offset), right.ptr, right.size);
		offset += right.size;
		memcpy(static_cast<void*>(base + offset), left.ptr, left.size);
		offset += left.size;
		memcpy(static_cast<void*>(base + offset), top.ptr, top.size);
		offset += top.size;
		memcpy(static_cast<void*>(base + offset), bottom.ptr, bottom.size);
		offset += bottom.size;
		memcpy(static_cast<void*>(base + offset), front.ptr, front.size);
		offset += front.size;
		memcpy(static_cast<void*>(base + offset), back.ptr, back.size);
	}

	void getFaceExtent(uint32_t& outWidth, uint32_t& outHeight) const
	{
		outWidth = back.width;
		outHeight = back.height;
	}

	Cubemap(const Cubemap& other) = delete;
	Cubemap& operator=(const Cubemap& other) = delete;

	Cubemap& operator=(Cubemap&& other) noexcept
	{
		if (this == &other) return *this;

		this->back = std::move(other.back);
		this->front = std::move(other.front);
		this->top = std::move(other.top);
		this->bottom = std::move(other.bottom);
		this->left = std::move(other.left);
		this->right = std::move(other.right);

		return *this;
	}
};