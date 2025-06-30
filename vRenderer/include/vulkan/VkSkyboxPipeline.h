#pragma once

#include "VulkanUtils.h"
#include "VkShaderManager.h"
#include "VkSetLayoutFactory.h"

using namespace VkUtils;

class VkSkyboxPipeline
{
public:

	VkSkyboxPipeline(VkRenderPass renderPass, VkContext context)
	{
		this->renderPass = renderPass;
		this->context = context;
		describe();
	}

	void cmdBind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	VkPipelineLayout getLayout() const
	{
		return layout;
	}

	~VkSkyboxPipeline()
	{
		//cleanup();
	}


	void cleanup()
	{
		vkDestroyPipelineLayout(context.logicalDevice, layout, nullptr);
		vkDestroyPipeline(context.logicalDevice, pipeline, nullptr);
	}

private:

	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkRenderPass renderPass;
	VkContext context;

	VkPushConstantRange pushConstantRange;

	void describe()
	{
		std::string key = "skybox";
		auto shaderStages = VkShaderManager::instance().getShaderStage(VkShaderManager::RenderPass::FIRST, &key);

		// DEFINING VERTEX ATTRIBUTES LAYOUT
		VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
		{
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(glm::vec3);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			std::array<VkVertexInputAttributeDescription, 1> attributes;
			attributes[0].binding = 0;										// should be same as above
			attributes[0].location = 0;
			attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributes[0].offset = 0;

			// VERTEX INPUT
			vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputCreateInfo.vertexBindingDescriptionCount = 1;
			vertexInputCreateInfo.pVertexBindingDescriptions = &bindingDescription;					// list of vertex binding descriptions (data spacing/stride information)
			vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
			vertexInputCreateInfo.pVertexAttributeDescriptions = attributes.data();				// list of vertex attribute descriptions (data format and where to bind to/from)
		}

		// INPUT ASSEMBLY
		// defines how vertex data is perceived (topology)
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		{
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;
		}

		// VIEWPORT & SCISSORS
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

		/*
		// DYNAMIC STATES
		// Dynamic states to enable
		std::vector<VkDynamicState> dynamicStatesEnables;
		dynamicStatesEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);	// allows to change viewport on runtime using vkCmdSetViewport
		dynamicStatesEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
		dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStatesEnables.size());
		dynamicStateCreateInfo.pDynamicStates = dynamicStatesEnables.data();
		*/

		// RASTERIZER
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

		// MULTI SAMPLING
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

		// PIPELINE LAYOUT SETUP (DESCRIPTORS AND PUSH CONSTANTS LAYOUT)
		{
			std::array<VkDescriptorSetLayout, 2> setLayouts = {
				VkSetLayoutFactory::instance().getSetLayout(DESC_SET_LAYOUT::CAMERA),
				createDescriptorSetLayout(1, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, context) 
			};

			// Defines push constant values
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			pushConstantRange.offset = 0;
			pushConstantRange.size = sizeof(PushConstant);

			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
			pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.setLayoutCount = 2;
			pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();
			pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
			pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

			// Create Pipeline Layout
			VkResult result = vkCreatePipelineLayout(context.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &layout);
			if (result != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create Pipeline Layout!");
			}
		}

		// DEPTH TESTING AND STENCIL TESTING SETUP
		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
		{
			depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

			depthStencilCreateInfo.depthTestEnable = VK_FALSE;
			depthStencilCreateInfo.depthWriteEnable = VK_FALSE;
			depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;

			depthStencilCreateInfo.stencilTestEnable = VK_TRUE;
			VkStencilOpState stencilState = {};
			stencilState.failOp = VK_STENCIL_OP_KEEP;
			stencilState.depthFailOp = VK_STENCIL_OP_KEEP;
			stencilState.passOp = VK_STENCIL_OP_REPLACE;
			stencilState.compareOp = VK_COMPARE_OP_ALWAYS;
			stencilState.reference = 1;
			stencilState.compareMask = 0xFF;
			stencilState.writeMask = 0xFF;
			depthStencilCreateInfo.front = stencilState;
		}

		// -- GRAPHICS PIPELINE CREATION --
		VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.stageCount = 2;									// Number of shader stages
		pipelineCreateInfo.pStages = shaderStages.data();							// List of shader stages
		pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;		// All the fixed function pipeline states
		pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
		pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
		pipelineCreateInfo.pDynamicState = nullptr;
		pipelineCreateInfo.pRasterizationState = &rastCreateInfo;
		pipelineCreateInfo.pMultisampleState = &multisamplingCreateInfo;
		pipelineCreateInfo.pColorBlendState = &colorBlendingCreateInfo;
		pipelineCreateInfo.pDepthStencilState = &depthStencilCreateInfo;
		pipelineCreateInfo.layout = layout;							// Pipeline Layout pipeline should use
		pipelineCreateInfo.renderPass = renderPass;							// Render pass description the pipeline is compatible with
		pipelineCreateInfo.subpass = 0;							 			// Subpass of render pass to use with pipeline

		// Pipeline Derivatives : Can create multiple pipelines that derive from one another for optimisation
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;	// Existing pipeline to derive from...
		pipelineCreateInfo.basePipelineIndex = -1;				// or index of pipeline being created to derive from (in case creating multiple at once)

		// Create Main Pipeline
		VkResult result = vkCreateGraphicsPipelines(context.logicalDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline);
		if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create the Main Pipeline!");
		}
	}
};