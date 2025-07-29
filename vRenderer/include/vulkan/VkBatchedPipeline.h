#pragma once

#include "VkGraphicsPipelineBase.h"
#include "VkModelBatch.h"

class VkBatchedPipeline : public VkGraphicsPipelineBase
{
public:

	VkBatchedPipeline(VkRenderPass renderPass, VkContext context) :
		VkGraphicsPipelineBase(renderPass, context)
	{
		describe();
	}

protected:

	virtual void describe() override
	{
		VkShaderManager& inst = VkShaderManager::instance();
		std::string key = "batch";
		auto shaderStages = inst.getShaderStage(VkShaderManager::RenderPass::FIRST, &key);

			// Batched pipeline requires one more 
			VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {};
			// 4 per vertex (pos, color, normal, uv) + 4 per instance (4 colums of 4x4 matrix)
			constexpr int attributesCount = 12;
			std::array<VkVertexInputAttributeDescription, attributesCount> attributes;

			VkVertexInputBindingDescription perVertexBindingDesc = {};
			perVertexBindingDesc.binding = 0;
			perVertexBindingDesc.stride = sizeof(VkUtils::Vertex);
			perVertexBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			attributes[0].binding = 0;										// should be same as above
			attributes[0].location = 0;
			attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributes[0].offset = offsetof(VkUtils::Vertex, pos);
			attributes[1].binding = 0;										// should be same as above
			attributes[1].location = 1;
			attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributes[1].offset = offsetof(VkUtils::Vertex, color);
			attributes[2].binding = 0;										// should be same as above
			attributes[2].location = 2;
			attributes[2].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributes[2].offset = offsetof(VkUtils::Vertex, normal);
			attributes[3].binding = 0;										// should be same as above
			attributes[3].location = 3;
			attributes[3].format = VK_FORMAT_R32G32_SFLOAT;
			attributes[3].offset = offsetof(VkUtils::Vertex, uv);

			VkVertexInputBindingDescription perInstanceBindingDesc = {};
			perInstanceBindingDesc.binding = 1;
			perInstanceBindingDesc.stride = sizeof(VkModelBatch::InstanceData);
			perInstanceBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

			attributes[4].binding = 1;
			attributes[4].location = 4;
			attributes[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributes[4].offset = offsetof(VkModelBatch::InstanceData, transform) + sizeof(glm::vec4) * 0;
			attributes[5].binding = 1;
			attributes[5].location = 5;
			attributes[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributes[5].offset = offsetof(VkModelBatch::InstanceData, transform) + sizeof(glm::vec4) * 1;
			attributes[6].binding = 1;
			attributes[6].location = 6;
			attributes[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributes[6].offset = offsetof(VkModelBatch::InstanceData, transform) + sizeof(glm::vec4) * 2;
			attributes[7].binding = 1;
			attributes[7].location = 7;
			attributes[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributes[7].offset = offsetof(VkModelBatch::InstanceData, transform) + sizeof(glm::vec4) * 3;

			attributes[8].binding = 1;
			attributes[8].location = 8;
			attributes[8].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributes[8].offset = offsetof(VkModelBatch::InstanceData, normalMat) + sizeof(glm::vec4) * 0;
			attributes[9].binding = 1;
			attributes[9].location = 9;
			attributes[9].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributes[9].offset = offsetof(VkModelBatch::InstanceData, normalMat) + sizeof(glm::vec4) * 1;
			attributes[10].binding = 1;
			attributes[10].location = 10;
			attributes[10].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributes[10].offset = offsetof(VkModelBatch::InstanceData, normalMat) + sizeof(glm::vec4) * 2;
			attributes[11].binding = 1;
			attributes[11].location = 11;
			attributes[11].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributes[11].offset = offsetof(VkModelBatch::InstanceData, normalMat) + sizeof(glm::vec4) * 3;

			std::array<VkVertexInputBindingDescription, 2> bindingDescriptions{
				perVertexBindingDesc,
				perInstanceBindingDesc
			};

				// VERTEX INPUT
	vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputCreateInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
	vertexInputCreateInfo.pVertexAttributeDescriptions = attributes.data();

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
			VkSetLayoutFactory& inst = VkSetLayoutFactory::instance();
			std::array<VkDescriptorSetLayout, 5> setLayouts = {
				inst.getSetLayout(DESC_SET_LAYOUT::CAMERA),
				inst.getSetLayout(DESC_SET_LAYOUT::MATERIAL_SAMPLER),
				inst.getSetLayout(DESC_SET_LAYOUT::MATERIAL_UNIFORM),
				inst.getSetLayout(DESC_SET_LAYOUT::LIGHT),
				inst.getSetLayout(DESC_SET_LAYOUT::DYNAMIC_COLOR),
			};

			// Defines push constant values
			//pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			//pushConstantRange.offset = 0;
			//pushConstantRange.size = sizeof(PushConstant);

			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
			pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.setLayoutCount = setLayouts.size();
			pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
			pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

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

			depthStencilCreateInfo.depthTestEnable = VK_TRUE;
			depthStencilCreateInfo.depthWriteEnable = VK_TRUE;
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