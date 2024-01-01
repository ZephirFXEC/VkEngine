#include <pch.hpp>

#include "vkEnginePipeline.hpp"

#include "logger.hpp"

namespace vke {
VkEnginePipeline::VkEnginePipeline(VkEngineDevice& device, const std::string& vertShader, const std::string& fragShader,
                                   const PipelineConfigInfo& configInfo)
    : mDevice(device) {
	createGraphicsPipeline(vertShader, fragShader, configInfo);
}

VkEnginePipeline::~VkEnginePipeline() {
	vkDestroyShaderModule(mDevice.getDevice(), mShaders.pVertShaderModule, nullptr);
	vkDestroyShaderModule(mDevice.getDevice(), mShaders.pFragShaderModule, nullptr);

	vkDestroyPipeline(mDevice.getDevice(), pGraphicsPipeline, nullptr);
}

void VkEnginePipeline::bind(const VkCommandBuffer* const commandBuffer) const {
	vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pGraphicsPipeline);
}

void VkEnginePipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo) {
	configInfo.viewportInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
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
	    .depthBiasConstantFactor = 0.0f,
	    // Optional
	    .depthBiasClamp = 0.0f,
	    // Optional
	    .depthBiasSlopeFactor = 0.0f,
	    // Optional
	    .lineWidth = 1.0f,
	};

	configInfo.multisampleInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
	    .sampleShadingEnable = VK_FALSE,
	    .minSampleShading = 1.0f,
	    // Optional
	    .pSampleMask = nullptr,
	    // Optional
	    .alphaToCoverageEnable = VK_FALSE,
	    // Optional
	    .alphaToOneEnable = VK_FALSE,
	    // Optional
	};

	configInfo.colorBlendAttachment = {
	    .blendEnable = VK_FALSE,
	    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
	    // Optional
	    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
	    // Optional
	    .colorBlendOp = VK_BLEND_OP_ADD,
	    // Optional
	    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
	    // Optional
	    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
	    // Optional
	    .alphaBlendOp = VK_BLEND_OP_ADD,
	    // Optional
	    .colorWriteMask =
	        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};

	configInfo.colorBlendInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
	    .logicOpEnable = VK_FALSE,
	    .logicOp = VK_LOGIC_OP_COPY,
	    // Optional
	    .attachmentCount = 1,
	    .pAttachments = &configInfo.colorBlendAttachment,
	    .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f},
	    // Optional
	};

	configInfo.depthStencilInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
	    .depthTestEnable = VK_TRUE,
	    .depthWriteEnable = VK_TRUE,
	    .depthCompareOp = VK_COMPARE_OP_LESS,
	    .depthBoundsTestEnable = VK_FALSE,
	    .stencilTestEnable = VK_FALSE,
	    .front = {},
	    // Optional
	    .back = {},
	    // Optional
	    .minDepthBounds = 0.0f,
	    // Optional
	    .maxDepthBounds = 1.0f,
	    // Optional
	};

	constexpr u32 dynamicStateCount = 2;
	configInfo.pDynamicStateEnables = new VkDynamicState[dynamicStateCount]{
	    VK_DYNAMIC_STATE_VIEWPORT,
	    VK_DYNAMIC_STATE_SCISSOR,
	};

	configInfo.dynamicStateInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
	    .flags = 0,
	    .dynamicStateCount = dynamicStateCount,
	    .pDynamicStates = configInfo.pDynamicStateEnables,
	};
}

char* VkEnginePipeline::readFile(const std::string& filename, size_t& bufferSize) {
	std::ifstream file{filename, std::ios::ate | std::ios::binary};

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file: " + filename);
	}

	bufferSize = static_cast<size_t>(file.tellg());
	auto* const buffer = new char[bufferSize];

	file.seekg(0);
	file.read(buffer, static_cast<u32>(bufferSize));
	file.close();

	return buffer;
}

void VkEnginePipeline::createGraphicsPipeline(const std::string& vertShader, const std::string& fragShader,
                                              const PipelineConfigInfo& configInfo) {
	assert(configInfo.pipelineLayout != VK_NULL_HANDLE &&
	       "Cannot create graphics pipeline: no pipelineLayout provided in "
	       "configInfo");
	assert(configInfo.renderPass != VK_NULL_HANDLE &&
	       "Cannot create graphics pipeline: no renderPass provided in "
	       "configInfo");

	size_t vertShaderSize = 0;
	size_t fragShaderSize = 0;
	const auto* const vertShaderCode = readFile(vertShader, vertShaderSize);
	const auto* const fragShaderCode = readFile(fragShader, fragShaderSize);

	createShaderModule(vertShaderCode, vertShaderSize, &mShaders.pVertShaderModule);
	createShaderModule(fragShaderCode, fragShaderSize, &mShaders.pFragShaderModule);

	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{
	    {{.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	      .pNext = nullptr,
	      .flags = 0,
	      .stage = VK_SHADER_STAGE_VERTEX_BIT,
	      .module = mShaders.pVertShaderModule,
	      .pName = "main",
	      .pSpecializationInfo = nullptr

	     },
	     {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	      .pNext = nullptr,
	      .flags = 0,
	      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
	      .module = mShaders.pFragShaderModule,
	      .pName = "main",
	      .pSpecializationInfo = nullptr}}};

	const auto bindingDescriptions = VkEngineModel::Vertex::getBindingDescriptions();
	const auto attributeDescriptions = VkEngineModel::Vertex::getAttributeDescriptions();

	constexpr u32 bindingDescriptionCount = 1;
	constexpr u32 attributeDescriptionCount = 2;

	const VkPipelineVertexInputStateCreateInfo vertexInputInfo{
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	    .vertexBindingDescriptionCount = bindingDescriptionCount,
	    .pVertexBindingDescriptions = bindingDescriptions->data(),
	    .vertexAttributeDescriptionCount = attributeDescriptionCount,
	    .pVertexAttributeDescriptions = attributeDescriptions->data(),
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
	    .pDepthStencilState = &configInfo.depthStencilInfo,
	    .pColorBlendState = &configInfo.colorBlendInfo,
	    .pDynamicState = &configInfo.dynamicStateInfo,
	    .layout = configInfo.pipelineLayout,
	    .renderPass = configInfo.renderPass,
	    .subpass = configInfo.subpass,
	    .basePipelineHandle = VK_NULL_HANDLE,
	    .basePipelineIndex = -1,
	};

	VK_CHECK(
	    vkCreateGraphicsPipelines(mDevice.getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pGraphicsPipeline));

	delete[] vertShaderCode;
	delete[] fragShaderCode;
}

void VkEnginePipeline::createShaderModule(const char* const code, const size_t codeSize,
                                          VkShaderModule* shaderModule) const {
	const VkShaderModuleCreateInfo createInfo = {.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
	                                             .codeSize = codeSize,
	                                             .pCode = reinterpret_cast<const u32*>(code)};

	VK_CHECK(vkCreateShaderModule(mDevice.getDevice(), &createInfo, nullptr, shaderModule));
}
}  // namespace vke