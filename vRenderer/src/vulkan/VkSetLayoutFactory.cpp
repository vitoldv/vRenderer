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
	layoutDefinitions[DESC_SET_LAYOUT::MATERIAL_SAMPLER] = createDescriptorSetLayout(4, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, context);
	layoutDefinitions[DESC_SET_LAYOUT::MATERIAL_UNIFORM] = createDescriptorSetLayout(1, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, context);
	layoutDefinitions[DESC_SET_LAYOUT::LIGHT] = createDescriptorSetLayout(1, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, context);
	layoutDefinitions[DESC_SET_LAYOUT::DYNAMIC_COLOR] = createDescriptorSetLayout(1, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, context);
	layoutDefinitions[DESC_SET_LAYOUT::CUBEMAP_SAMPLER] = createDescriptorSetLayout(1, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, context);
	layoutDefinitions[DESC_SET_LAYOUT::SECOND_PASS_INPUT] = createDescriptorSetLayout(2, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, context);

}

VkDescriptorSetLayout VkSetLayoutFactory::getSetLayout(DESC_SET_LAYOUT layout)
{
	return layoutDefinitions[layout];
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
