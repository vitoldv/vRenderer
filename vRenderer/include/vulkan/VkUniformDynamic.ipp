template<typename T>
inline VkUniformDynamic<T>::VkUniformDynamic(uint32_t descriptorSetIndex, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkContext context) :
	descriptorSetIndex(descriptorSetIndex),
	imageCount(context.imageCount)
{
	this->context = context;
	this->descriptorSetLayout = descriptorSetLayout;
	this->descriptorPool = descriptorPool;

	buffer.resize(imageCount);
	memory.resize(imageCount);
	descriptorSet.resize(imageCount);

	allocateDynamicBufferTransferSpace();
	create();
}

template<typename T>
inline VkUniformDynamic<T>::~VkUniformDynamic()
{
	// cleanup();
}

template<typename T>
inline void VkUniformDynamic<T>::allocateDynamicBufferTransferSpace()
{
	// Calculate alignment of single unit's data
	alignment = (sizeof(T) + context.minUniformBufferOffset - 1) & ~(context.minUniformBufferOffset - 1);

	// Create space in memory to hold dynamic buffer that is alighned to our required alignment and holds MAX_OBJECTS
	transferSpace = (T*)_aligned_malloc(alignment * MAX_OBJECTS, alignment);
}

template<typename T>
inline size_t VkUniformDynamic<T>::getBufferSize() const
{
	return alignment * MAX_OBJECTS;
}

/// <summary>
/// Updates the whole uniform buffer 
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="data"></param>
/// <param name="drawCount"></param>
template<typename T>
inline void VkUniformDynamic<T>::update(uint32_t imageIndex, const T* data, uint32_t drawCount)
{
	if (drawCount > 0)
	{
		for (int i = 0; i < drawCount; i++)
		{
			T* thisVal = (T*)((uint64_t)transferSpace + (i * alignment));
			*thisVal = *data;
			data++;
		}

		void* dataPtr;
		vkMapMemory(context.logicalDevice, memory[imageIndex], 0, getBufferSize(), 0, &dataPtr);
		memcpy(dataPtr, transferSpace, getBufferSize());
		vkUnmapMemory(context.logicalDevice, memory[imageIndex]);
	}
}

template<typename T>
inline void VkUniformDynamic<T>::updateAll(const T* data, uint32_t drawCount)
{
	for (int i = 0; i < imageCount; i++)
	{
		update(i, data, drawCount);
	}
}

template<typename T>
inline void VkUniformDynamic<T>::create()
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
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = buffer[i];
			bufferInfo.offset = 0;
			bufferInfo.range = alignment;
			VkWriteDescriptorSet setWrite = {};
			setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			setWrite.dstSet = descriptorSet[i];
			setWrite.dstBinding = 0;
			setWrite.dstArrayElement = 0;
			setWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			setWrite.descriptorCount = 1;
			setWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(context.logicalDevice, 1, &setWrite, 0, nullptr);
		}
	}
}

template<typename T>
inline void VkUniformDynamic<T>::cmdBind(uint32_t imageIndex, uint32_t drawIndex, VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	uint32_t dynamicOffset = static_cast<uint32_t>(alignment) * drawIndex;
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
		descriptorSetIndex, 1, &descriptorSet[imageIndex], 1, &dynamicOffset);
}

template<typename T>
inline void VkUniformDynamic<T>::cleanup()
{
	_aligned_free(transferSpace);

	for (int i = 0; i < imageCount; i++)
	{
		vkDestroyBuffer(context.logicalDevice, buffer[i], nullptr);
		vkFreeMemory(context.logicalDevice, memory[i], nullptr);
	}
}

template<typename T>
inline const VkDescriptorSetLayout VkUniformDynamic<T>::getDescriptorLayout() const
{
	return descriptorSetLayout;
}