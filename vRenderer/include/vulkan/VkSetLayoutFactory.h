#pragma once

#include <vector>
#include <unordered_map>
#include <map>

#include "Singleton.h"
#include "VulkanUtils.h"

using namespace VkUtils;

enum class DESC_SET_LAYOUT
{
	CAMERA,
	MATERIAL_SAMPLER,
	MATERIAL_UNIFORM,
	LIGHT,		
	DYNAMIC_COLOR,
	CUBEMAP_SAMPLER 
};

class VkSetLayoutFactory : public Singleton<VkSetLayoutFactory> 
{
	friend class Singleton<VkSetLayoutFactory>;

public:

	VkDescriptorSetLayout getSetLayout(DESC_SET_LAYOUT layout);
	uint32_t getSetIndexForLayout(DESC_SET_LAYOUT layout);
	void cleanup();

protected:

	VkSetLayoutFactory(VkContext context);
	~VkSetLayoutFactory();

private:

	VkContext context;
	std::map<DESC_SET_LAYOUT, VkDescriptorSetLayout> layoutDefinitions;

	void createLayoutDefinitions();	
};