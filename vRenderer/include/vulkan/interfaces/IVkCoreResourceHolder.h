#pragma once

/// <summary>
/// Defines a class that allocates a memory or creates an object within a Vulkan instance
/// </summary>
class IVkCoreResourceHolder
{
public:

	/// <summary>
	/// Release all Vulkan resources
	/// </summary>
	virtual void cleanup() = 0;
};