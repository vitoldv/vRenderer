#pragma once

#include <vector>
#include <memory>
#include "VulkanUtils.h"

using namespace VkUtils;

template<typename T>
class VkUniform
{
public:

	VkUniform(VkShaderStageFlagBits shaderStageFlags, VkContext context);
	~VkUniform();

	void update(const T& data);
	void cmdBind(uint32_t setNumber, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	const VkDescriptorSetLayout getDescriptorLayout() const;

	void cleanup();

private:
	//std::vector<VkBuffer> lightUniformBuffers;
	//std::vector<VkDeviceMemory> lightUniformMemory;
	//VkDescriptorSetLayout vkLightDescriptorSetLayout;
	//std::vector<VkDescriptorSet> vkLightDescriptorSets;

	VkBuffer buffer;
	VkDeviceMemory memory;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;

	VkShaderStageFlagBits shaderStage;
	VkContext context;

	void create();
};

template<typename T>
inline VkUniform<T>::VkUniform(VkShaderStageFlagBits shaderStageFlags, VkContext context)
{
	//lightUniformBuffers.resize(frameCount);
	//lightUniformMemory.resize(frameCount);
	//vkLightDescriptorSetLayout = VK_NULL_HANDLE;
	//vkLightDescriptorSets.resize(frameCount);
	this->context = context;
	shaderStage = shaderStageFlags;
	create();
}

template<typename T>
inline VkUniform<T>::~VkUniform()
{
	//cleanup();
}

template<typename T>
inline void VkUniform<T>::update(const T& data)
{
	// Copy uniform data
	void* dataPtr;
	vkMapMemory(context.logicalDevice, memory, 0, sizeof(T), 0, &dataPtr);
	memcpy(dataPtr, &data, sizeof(T));
	vkUnmapMemory(context.logicalDevice, memory);
}

template<typename T>
inline void VkUniform<T>::cmdBind(uint32_t setNumber, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
		setNumber, 1, &descriptorSet, 0, nullptr);
}

template<typename T>
inline const VkDescriptorSetLayout VkUniform<T>::getDescriptorLayout() const
{
	return descriptorSetLayout;
}

template<typename T>
inline void VkUniform<T>::cleanup()
{
	vkDestroyBuffer(context.logicalDevice, buffer, nullptr);
	vkFreeMemory(context.logicalDevice, memory, nullptr);
	vkDestroyDescriptorSetLayout(context.logicalDevice, descriptorSetLayout, nullptr);
}

template<typename T>
inline void VkUniform<T>::create()
{
	// LAYOUT CREATION
	{
		VkDescriptorSetLayoutBinding binding;
		binding.binding = 0;												// bindings specified in shader
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;			// type of descriptor (simple uniform in this case)
		binding.descriptorCount = 1;										// number of binded values
		binding.stageFlags = shaderStage;									// specifies shader stage
		binding.pImmutableSamplers = nullptr;								// for textures

		// Create descriptor set layout with given bindings
		VkDescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = 1;
		createInfo.pBindings = &binding;

		VkResult result = vkCreateDescriptorSetLayout(context.logicalDevice, &createInfo, nullptr, &this->descriptorSetLayout);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create uniform dessciptor layout");
		}
	}

	// Buffer creation
	{
		// Buffer size should be the size of data we pass as uniforms
		VkDeviceSize bufferSize = sizeof(T);
		createBuffer(context.physicalDevice, context.logicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer, &memory);
	}

	// Descriptor set allocation
	{
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = context.uniformDescriptorPool;						// pool to allocate sets from
		allocInfo.descriptorSetCount = 1;						// number of sets to allocate
		allocInfo.pSetLayouts = &descriptorSetLayout;								// layouts to use to allocate sets (1:1)

		VkResult result = vkAllocateDescriptorSets(context.logicalDevice, &allocInfo, &descriptorSet);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate Descriptor set.");
		}
	}

	// Bind buffers to descriptor sets
	{
		VkDescriptorBufferInfo descriptorBufferInfo = {};
		descriptorBufferInfo.buffer = buffer;
		descriptorBufferInfo.offset = 0;
		descriptorBufferInfo.range = sizeof(T);

		// LEFT FOR REFERENCE ON DYNAMIC UNIFORM BUFFERS
		// //DYNAMIC UNIFORM BUFFER (used for passing specific model's transform)
		//VkDescriptorBufferInfo modelBufferInfo = {};
		//modelBufferInfo.buffer = uniformBuffersDynamic[i];
		//modelBufferInfo.offset = 0;
		//modelBufferInfo.range = modelUniformAlignment;
		//VkWriteDescriptorSet modelSetWrite = {};
		//modelSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		//modelSetWrite.dstSet = this->vkDescriptorSets[i];
		//modelSetWrite.dstBinding = 1;
		//modelSetWrite.dstArrayElement = 0;
		//modelSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		//modelSetWrite.descriptorCount = 1;
		//modelSetWrite.pBufferInfo = &modelBufferInfo;
		//std::vector<VkWriteDescriptorSet> setWrites = { vpSetWrite, modelSetWrite };

		VkWriteDescriptorSet setWrite = {};
		setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrite.dstSet = descriptorSet;
		setWrite.dstBinding = 0;											// matches with binding on layout in shader
		setWrite.dstArrayElement = 0;
		setWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		setWrite.descriptorCount = 1;
		setWrite.pBufferInfo = &descriptorBufferInfo;

		vkUpdateDescriptorSets(context.logicalDevice, 1, &setWrite, 0, nullptr);
	}
}
