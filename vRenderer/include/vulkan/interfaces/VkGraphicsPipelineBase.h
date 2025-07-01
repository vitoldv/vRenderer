#pragma once

#include "VulkanCore.h"
#include "VulkanUtils.h"
#include "IVkCoreResourceHolder.h"
#include "VkShaderManager.h"
#include "VkSetLayoutFactory.h"

using namespace VkUtils;

/// <summary>
/// A base class for a graphics pipeline.
/// </summary>
class VkGraphicsPipelineBase : public IVkCoreResourceHolder
{
public:

	VkGraphicsPipelineBase(VkRenderPass renderPass, VkContext context)
	{
		this->renderPass = renderPass;
		this->context = context;
	}

	virtual ~VkGraphicsPipelineBase() {}

	void cmdBind(VkCommandBuffer commandBuffer) const
	{
		bind_internal();
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	inline VkPipelineLayout getLayout() const
	{
		return layout;
	}

	void cleanup()
	{
		cleanup_internal();
		vkDestroyPipelineLayout(context.logicalDevice, layout, nullptr);
		vkDestroyPipeline(context.logicalDevice, pipeline, nullptr);
	}

protected:

	/// <summary>
	/// Binds addional descendant's resources
	/// </summary>
	virtual void bind_internal() const {}

	/// <summary>
	/// Cleans additional descendant's resources
	/// </summary>
	virtual void cleanup_internal() {}

	/// <summary>
	/// CreatesVkPipeline object and ultimately describes the graphics pipeline.
	/// </summary>
	virtual void describe() = 0;

	VkPipeline pipeline			= VK_NULL_HANDLE;
	VkPipelineLayout layout		= VK_NULL_HANDLE;
	VkRenderPass renderPass		= VK_NULL_HANDLE;
	VkContext context;
};