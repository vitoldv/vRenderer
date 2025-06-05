#include "VkUniform.hpp"
#pragma once
template<typename T>
inline VkUniform<T>::VkUniform(uint32_t imageCount, uint32_t descriptorSetIndex, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkContext context) :
	descriptorSetIndex(descriptorSetIndex),
	imageCount(imageCount)
{
	this->context = context;
	this->descriptorSetLayout = descriptorSetLayout;
	this->descriptorPool = descriptorPool;

	buffer.resize(imageCount);
	memory.resize(imageCount);
	descriptorSet.resize(imageCount);

	create();
}

template<typename T>
inline VkUniform<T>::~VkUniform()
{
	//cleanup();
}

template<typename T>
inline size_t VkUniform<T>::getBufferSize()
{
	return sizeof(T);
}

template<typename T>
inline void VkUniform<T>::update(uint32_t imageIndex, const T& data)
{
	if (imageIndex < imageCount)
	{
		// Copy uniform data
		void* dataPtr;
		vkMapMemory(context.logicalDevice, memory[imageIndex], 0, getBufferSize(), 0, &dataPtr);
		memcpy(dataPtr, &data, getBufferSize());
		vkUnmapMemory(context.logicalDevice, memory[imageIndex]);
	}
}

template<typename T>
inline void VkUniform<T>::updateAll(const T& data)
{
	for (int i = 0; i < imageCount; i++)
	{
		update(i, data);
	}
}

template<typename T>
inline void VkUniform<T>::cmdBind(uint32_t imageIndex, uint32_t setNumber, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout) const
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
		setNumber, 1, &descriptorSet[imageIndex], 0, nullptr);
}

template<typename T>
inline void VkUniform<T>::cleanup()
{
	for (int i = 0; i < imageCount; i++)
	{
		vkDestroyBuffer(context.logicalDevice, buffer[i], nullptr);
		vkFreeMemory(context.logicalDevice, memory[i], nullptr);
	}
}

template<typename T>
inline void VkUniform<T>::create()
{
	for (int i = 0; i < imageCount; i++)
	{
		// Buffer creation
		{
			// Buffer size should be the size of data we pass as uniforms
			VkDeviceSize bufferSize = getBufferSize();
			createBuffer(context.physicalDevice, context.logicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer[i], &memory[i]);
		}

		// Descriptor set allocation
		{
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;						// pool to allocate sets from
			allocInfo.descriptorSetCount = 1;						// number of sets to allocate
			allocInfo.pSetLayouts = &descriptorSetLayout;								// layouts to use to allocate sets (1:1)

			VkResult result = vkAllocateDescriptorSets(context.logicalDevice, &allocInfo, &descriptorSet[i]);
			if (result != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to allocate Descriptor set.");
			}
		}

		// Bind buffers to descriptor sets
		{
			VkDescriptorBufferInfo descriptorBufferInfo = {};
			descriptorBufferInfo.buffer = buffer[i];
			descriptorBufferInfo.offset = 0;
			descriptorBufferInfo.range = getBufferSize();

			VkWriteDescriptorSet setWrite = {};
			setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			setWrite.dstSet = descriptorSet[i];
			setWrite.dstBinding = 0;											// matches with binding on layout in shader
			setWrite.dstArrayElement = 0;
			setWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			setWrite.descriptorCount = 1;
			setWrite.pBufferInfo = &descriptorBufferInfo;

			vkUpdateDescriptorSets(context.logicalDevice, 1, &setWrite, 0, nullptr);
		}
	}
}

