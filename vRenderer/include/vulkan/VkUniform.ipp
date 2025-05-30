#pragma once
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
inline size_t VkUniform<T>::getBufferSize()
{
	return sizeof(T);
}

template<typename T>
inline void VkUniform<T>::update(const T& data)
{
	// Copy uniform data
	void* dataPtr;
	vkMapMemory(context.logicalDevice, memory, 0, getBufferSize(), 0, &dataPtr);
	memcpy(dataPtr, &data, getBufferSize());
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
		VkDeviceSize bufferSize = getBufferSize();
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
		descriptorBufferInfo.range = getBufferSize();

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

