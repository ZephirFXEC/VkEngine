#include "vkEnginePipeline.hpp"

// std
#include <cassert>
#include <fstream>
#include <stdexcept>

namespace vke {
VkEnginePipeline::VkEnginePipeline(VkEngineDevice& device,
                                   const std::string& vertShader,
                                   const std::string& fragShader,
                                   const PipelineConfigInfo& configInfo)
    : mDevice(device) {
	createGraphicsPipeline(vertShader, fragShader, configInfo);
}

VkEnginePipeline::~VkEnginePipeline() {
	vkDestroyShaderModule(mDevice.device(), pVertShaderModule, nullptr);
	vkDestroyShaderModule(mDevice.device(), pFragShaderModule, nullptr);

	vkDestroyPipeline(mDevice.device(), pGraphicsPipeline, nullptr);
}

void VkEnginePipeline::bind(const VkCommandBuffer commandBuffer) const {
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
	                  pGraphicsPipeline);
}

void VkEnginePipeline::defaultPipelineConfigInfo(
    PipelineConfigInfo& configInfo) {
	configInfo.viewportInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
	    .viewportCount = 1,
	    .pViewports = nullptr,
	    .scissorCount = 1,
	    .pScissors = nullptr};

	configInfo.inputAssemblyInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	    .primitiveRestartEnable = VK_FALSE,
	};

	configInfo.rasterizationInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	    .depthClampEnable = VK_FALSE,
	    .rasterizerDiscardEnable = VK_FALSE,
	    .polygonMode = VK_POLYGON_MODE_FILL,
	    .cullMode = VK_CULL_MODE_BACK_BIT,
	    .frontFace = VK_FRONT_FACE_CLOCKWISE,
	    .depthBiasEnable = VK_FALSE,
	    .depthBiasConstantFactor = 0.0f,  // Optional
	    .depthBiasClamp = 0.0f,           // Optional
	    .depthBiasSlopeFactor = 0.0f,     // Optional
	    .lineWidth = 1.0f,
	};

	configInfo.multisampleInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
	    .sampleShadingEnable = VK_FALSE,
	    .minSampleShading = 1.0f,           // Optional
	    .pSampleMask = nullptr,             // Optional
	    .alphaToCoverageEnable = VK_FALSE,  // Optional
	    .alphaToOneEnable = VK_FALSE,       // Optional
	};

	configInfo.colorBlendAttachment = {
	    .blendEnable = VK_FALSE,
	    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,   // Optional
	    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,  // Optional
	    .colorBlendOp = VK_BLEND_OP_ADD,              // Optional
	    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,   // Optional
	    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,  // Optional
	    .alphaBlendOp = VK_BLEND_OP_ADD,              // Optional
	    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};

	configInfo.colorBlendInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
	    .logicOpEnable = VK_FALSE,
	    .logicOp = VK_LOGIC_OP_COPY,  // Optional
	    .attachmentCount = 1,
	    .pAttachments = &configInfo.colorBlendAttachment,
	    .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},  // Optional
	};

	configInfo.depthStencilInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
	    .depthTestEnable = VK_TRUE,
	    .depthWriteEnable = VK_TRUE,
	    .depthCompareOp = VK_COMPARE_OP_LESS,
	    .depthBoundsTestEnable = VK_FALSE,
	    .stencilTestEnable = VK_FALSE,
	    .front = {},             // Optional
	    .back = {},              // Optional
	    .minDepthBounds = 0.0f,  // Optional
	    .maxDepthBounds = 1.0f,  // Optional
	};

	configInfo.dynamicStateEnables = {
	    VK_DYNAMIC_STATE_VIEWPORT,
	    VK_DYNAMIC_STATE_SCISSOR,
	};

	configInfo.dynamicStateInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
	    .dynamicStateCount =
	        static_cast<uint32_t>(configInfo.dynamicStateEnables.size()),
	    .pDynamicStates = configInfo.dynamicStateEnables.data(),
	    .flags = 0,
	};
}

std::vector<char> VkEnginePipeline::readFile(const std::string& filename) {
	std::ifstream file{filename, std::ios::ate | std::ios::binary};

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file: " + filename);
	}

	std::vector<char> buffer(static_cast<size_t>(file.tellg()));

	file.seekg(0);
	file.read(buffer.data(), static_cast<long>(buffer.size()));
	file.close();

	return buffer;
}

void VkEnginePipeline::createGraphicsPipeline(
    const std::string& vertShader, const std::string& fragShader,
    const PipelineConfigInfo& configInfo) {
	assert(configInfo.pipelineLayout != VK_NULL_HANDLE &&
	       "Cannot create graphics pipeline: no pipelineLayout provided in "
	       "configInfo");
	assert(configInfo.renderPass != VK_NULL_HANDLE &&
	       "Cannot create graphics pipeline: no renderPass provided in "
	       "configInfo");

	const auto vertShaderCode = readFile(vertShader);
	const auto fragShaderCode = readFile(fragShader);

	createShaderModule(vertShaderCode, &pVertShaderModule);
	createShaderModule(fragShaderCode, &pFragShaderModule);

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{
	    {{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	      .pNext = nullptr,
	      .flags = 0,
	      .stage = VK_SHADER_STAGE_VERTEX_BIT,
	      .module = pVertShaderModule,
	      .pName = "main",
	      .pSpecializationInfo = nullptr

	     },
	     {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	      .pNext = nullptr,
	      .flags = 0,
	      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
	      .module = pFragShaderModule,
	      .pName = "main",
	      .pSpecializationInfo = nullptr}}};

	const auto bindingDescriptions =
	    VkEngineModel::Vertex::getBindingDescriptions();
	const auto attributeDescriptions =
	    VkEngineModel::Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	    .vertexBindingDescriptionCount =
	        static_cast<uint32_t>(bindingDescriptions.size()),
	    .pVertexBindingDescriptions = bindingDescriptions.data(),
	    .vertexAttributeDescriptionCount =
	        static_cast<uint32_t>(attributeDescriptions.size()),
	    .pVertexAttributeDescriptions = attributeDescriptions.data(),
	};

	const VkGraphicsPipelineCreateInfo pipelineInfo{
	    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
	    .stageCount = 2,
	    .pStages = shaderStages.data(),
	    .pVertexInputState = &vertexInputInfo,
	    .pInputAssemblyState = &configInfo.inputAssemblyInfo,
	    .pViewportState = &configInfo.viewportInfo,
	    .pRasterizationState = &configInfo.rasterizationInfo,
	    .pMultisampleState = &configInfo.multisampleInfo,
	    .pColorBlendState = &configInfo.colorBlendInfo,
	    .pDepthStencilState = &configInfo.depthStencilInfo,
	    .pDynamicState = &configInfo.dynamicStateInfo,
	    .layout = configInfo.pipelineLayout,
	    .renderPass = configInfo.renderPass,
	    .subpass = configInfo.subpass,
	    .basePipelineIndex = -1,
	    .basePipelineHandle = VK_NULL_HANDLE,
	};

	if (vkCreateGraphicsPipelines(mDevice.device(), VK_NULL_HANDLE, 1,
	                              &pipelineInfo, nullptr,
	                              &pGraphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline");
	}
}

void VkEnginePipeline::createShaderModule(const std::vector<char>& code,
                                          VkShaderModule* shaderModule) const {
	const VkShaderModuleCreateInfo createInfo = {
	    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
	    .codeSize = code.size(),
	    .pCode = reinterpret_cast<const uint32_t*>(code.data())};

	if (vkCreateShaderModule(mDevice.device(), &createInfo, nullptr,
	                         shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Shader Module");
	}
}
}  // namespace vke
