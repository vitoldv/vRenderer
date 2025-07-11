#pragma once

#include "VkGraphicsPipelineBase.h"

class VkSecondPassPipeline : public VkGraphicsPipelineBase
{
public:

	VkSecondPassPipeline(VkRenderPass renderPass, VkContext context) :
		VkGraphicsPipelineBase(renderPass, context)
	{
		describe();
	}

protected:

	virtual void describe() override
	{
		VkShaderManager& inst = VkShaderManager::instance();
		auto shaderStages = inst.getShaderStage(VkShaderManager::RenderPass::SECOND);

		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
		vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
		vertexInputCreateInfo.pVertexBindingDescriptions = nullptr;
		vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
		vertexInputCreateInfo.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		{
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;
		}

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
		{
			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)context.imageExtent.width;
			viewport.height = (float)context.imageExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.offset = { 0,0 };
			scissor.extent = context.imageExtent;

			viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportStateCreateInfo.viewportCount = 1;
			viewportStateCreateInfo.pViewports = &viewport;
			viewportStateCreateInfo.scissorCount = 1;
			viewportStateCreateInfo.pScissors = &scissor;
		}

		VkPipelineRasterizationStateCreateInfo rastCreateInfo = {};
		{
			rastCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rastCreateInfo.depthClampEnable = VK_FALSE;
			rastCreateInfo.rasterizerDiscardEnable = VK_FALSE;
			rastCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
			rastCreateInfo.lineWidth = 1.0f;
			rastCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
			rastCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rastCreateInfo.depthBiasEnable = VK_FALSE;				// whether to add depth bias to fragments (good for stopping "shadow acne" in shadow mapping)
		}

		VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo = {};
		{
			multisamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisamplingCreateInfo.sampleShadingEnable = VK_FALSE;
			multisamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		}

		// COLOR SETUP DEFINITION (blending etc.)
		VkPipelineColorBlendStateCreateInfo colorBlendingCreateInfo = {};
		{
			// BLENDING
			// Blending decides how to blend a new color being written to a fragment, with the old value

			// Blend Attachment State (how blending is handled)
			VkPipelineColorBlendAttachmentState colorState = {};
			colorState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT	// colors to apply blending to
				| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorState.blendEnable = VK_TRUE;													// Enable blending

			// Blending uses equation: (srcColorBlendFactor * new color) colorBlendOp (dstColorBlendFactor * old color)
			colorState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorState.colorBlendOp = VK_BLEND_OP_ADD;

			// Summarised: (VK_BLEND_FACTOR_SRC_ALPHA * new color) + (VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA * old color)
			//			   (new color alpha * new color) + ((1 - new color alpha) * old color)

			colorState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorState.alphaBlendOp = VK_BLEND_OP_ADD;
			// Summarised: (1 * new alpha) + (0 * old alpha) = new alpha

			colorBlendingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendingCreateInfo.logicOpEnable = VK_FALSE;
			colorBlendingCreateInfo.attachmentCount = 1;
			colorBlendingCreateInfo.pAttachments = &colorState;
		}

		// Create new pipeline layout
		VkPipelineLayoutCreateInfo secondPipelineLayoutCreateInfo = {};
		{
			VkSetLayoutFactory& layoutFactor = VkSetLayoutFactory::instance();
			std::array<VkDescriptorSetLayout, 2> setLayouts = {
				layoutFactor.getSetLayout(DESC_SET_LAYOUT::SECOND_PASS_INPUT),
				layoutFactor.getSetLayout(DESC_SET_LAYOUT::POST_PROCESSING_FEATURES)
			};
			secondPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			secondPipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
			secondPipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();
			secondPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
			secondPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

			VkResult result = vkCreatePipelineLayout(context.logicalDevice, &secondPipelineLayoutCreateInfo, nullptr, &layout);
			if (result != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create second pipeline layout.");
			}
		}

		// Don't want to write to depth buffer
		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
		depthStencilCreateInfo.depthWriteEnable = VK_FALSE;

		// -- GRAPHICS PIPELINE CREATION --
		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		{
			pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineCreateInfo.stageCount = 2;									// Number of shader stages
			pipelineCreateInfo.pStages = shaderStages.data();							// List of shader stages
			pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
			pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
			pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
			pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
			pipelineCreateInfo.pDynamicState = nullptr;
			pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
			pipelineCreateInfo.pRasterizationState = &rastCreateInfo;
			pipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
			pipelineCreateInfo.layout = layout;							// Pipeline Layout pipeline should use
			pipelineCreateInfo.renderPass = renderPass;							// Render pass description the pipeline is compatible with
			pipelineCreateInfo.subpass = 1;							 			// Subpass of render pass to use with pipeline

			// Pipeline Derivatives : Can create multiple pipelines that derive from one another for optimisation
			pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;	// Existing pipeline to derive from...
			pipelineCreateInfo.basePipelineIndex = -1;				// or index of pipeline being created to derive from (in case creating multiple at once)							// second subpass
		}

		// Create second pipeline
		VkResult result = vkCreateGraphicsPipelines(context.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create second graphics pipeline.");
		}
	}
};