#include "VkSetLayoutFactory.h"

VkSetLayoutFactory::VkSetLayoutFactory(VkContext context): 
	context(context)
{
	createLayoutDefinitions();
}

VkSetLayoutFactory::~VkSetLayoutFactory()
{
	//cleanup();
}

void VkSetLayoutFactory::createLayoutDefinitions()
{

	layoutDefinitions[DESC_SET_LAYOUT::CAMERA] = createDescriptorSetLayout(1, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, context);
	layoutDefinitions[DESC_SET_LAYOUT::MATERIAL] = createDescriptorSetLayout(4, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, context);
	layoutDefinitions[DESC_SET_LAYOUT::MATERIAL_2] = createDescriptorSetLayout(1, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, context);
	layoutDefinitions[DESC_SET_LAYOUT::LIGHT] = createDescriptorSetLayout(1, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, context);
	layoutDefinitions[DESC_SET_LAYOUT::DYNAMIC_COLOR] = createDescriptorSetLayout(1, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, context);
}


VkDescriptorSetLayout VkSetLayoutFactory::getSetLayout(DESC_SET_LAYOUT layout)
{
	return layoutDefinitions[layout];
}

uint32_t VkSetLayoutFactory::getSetIndexForLayout(DESC_SET_LAYOUT layout)
{
	return static_cast<uint32_t>(layout);
}

std::vector<VkDescriptorSetLayout> VkSetLayoutFactory::getLayouts()
{
	std::vector<VkDescriptorSetLayout> layouts;
	for (const auto& item : layoutDefinitions) {
		layouts.push_back(item.second);
	}
	return layouts;
}

void VkSetLayoutFactory::cleanup()
{
	for (auto& pair : layoutDefinitions)
	{
		auto& layout = pair.second;
		vkDestroyDescriptorSetLayout(context.logicalDevice, layout, nullptr);
	}
	layoutDefinitions.clear();
}
