#include "vkEnginePipeline.hpp"

// std
#include <cassert>
#include <fstream>
#include <stdexcept>
#include <vulkan/vulkan_structs.hpp>

namespace vke
{
VkEnginePipeline::VkEnginePipeline(VkEngineDevice& device,
								   const std::string& vertShader,
								   const std::string& fragShader,
								   const PipelineConfigInfo& configInfo)
	: mDevice(device)
{

	createGraphicsPipeline(vertShader, fragShader, configInfo);
}

VkEnginePipeline::~VkEnginePipeline()
{
	mDevice.device().destroyShaderModule(pVertShaderModule);
	mDevice.device().destroyShaderModule(pFragShaderModule);
	mDevice.device().destroyPipeline(pGraphicsPipeline);
}

void VkEnginePipeline::bind(const vk::CommandBuffer commandBuffer) const
{
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pGraphicsPipeline);
}

PipelineConfigInfo VkEnginePipeline::defaultPipelineConfigInfo(const uint32_t width,
															   const uint32_t height)
{

	PipelineConfigInfo configInfo{};
	configInfo.viewport.setHeight(static_cast<float>(height));
	configInfo.viewport.setWidth(static_cast<float>(width));
	configInfo.viewport.setMinDepth(0.0f);
	configInfo.viewport.setMaxDepth(1.0f);
	configInfo.scissor.setExtent({width, height});

	configInfo.inputAssemblyInfo.setTopology(vk::PrimitiveTopology::eTriangleList);
	configInfo.inputAssemblyInfo.setPrimitiveRestartEnable(VK_FALSE);

	configInfo.rasterizationInfo.setDepthClampEnable(VK_FALSE);
	configInfo.rasterizationInfo.setRasterizerDiscardEnable(VK_FALSE);
	configInfo.rasterizationInfo.setPolygonMode(vk::PolygonMode::eFill);
	configInfo.rasterizationInfo.setLineWidth(1.0f);
	configInfo.rasterizationInfo.setCullMode(vk::CullModeFlagBits::eNone);
	configInfo.rasterizationInfo.setFrontFace(vk::FrontFace::eCounterClockwise);
	configInfo.rasterizationInfo.setDepthBiasEnable(VK_FALSE);

	configInfo.multisampleInfo.setSampleShadingEnable(VK_FALSE);
	configInfo.multisampleInfo.setRasterizationSamples(vk::SampleCountFlagBits::e1);
	configInfo.multisampleInfo.setMinSampleShading(1.0f); // Optional
	configInfo.multisampleInfo.setPSampleMask(nullptr); // Optional
	configInfo.multisampleInfo.setAlphaToCoverageEnable(VK_FALSE); // Optional
	configInfo.multisampleInfo.setAlphaToOneEnable(VK_FALSE); // Optional

	configInfo.colorBlendAttachment.setColorWriteMask(
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
	configInfo.colorBlendAttachment.setBlendEnable(VK_FALSE);
	configInfo.colorBlendAttachment.setSrcColorBlendFactor(vk::BlendFactor::eOne); // Optional
	configInfo.colorBlendAttachment.setDstColorBlendFactor(vk::BlendFactor::eZero); // Optional
	configInfo.colorBlendAttachment.setColorBlendOp(vk::BlendOp::eAdd); // Optional
	configInfo.colorBlendAttachment.setSrcAlphaBlendFactor(vk::BlendFactor::eOne); // Optional
	configInfo.colorBlendAttachment.setDstAlphaBlendFactor(vk::BlendFactor::eZero); // Optional
	configInfo.colorBlendAttachment.setAlphaBlendOp(vk::BlendOp::eAdd); // Optional

	configInfo.colorBlendInfo.setLogicOpEnable(VK_FALSE);
	configInfo.colorBlendInfo.setLogicOp(vk::LogicOp::eCopy); // Optional
	configInfo.colorBlendInfo.setAttachmentCount(1);
	configInfo.colorBlendInfo.setPAttachments(&configInfo.colorBlendAttachment);
	configInfo.colorBlendInfo.setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f}); // Optional

	configInfo.depthStencilInfo.setDepthTestEnable(VK_TRUE);
	configInfo.depthStencilInfo.setDepthWriteEnable(VK_TRUE);
	configInfo.depthStencilInfo.setDepthCompareOp(vk::CompareOp::eLess);
	configInfo.depthStencilInfo.setDepthBoundsTestEnable(VK_FALSE);
	configInfo.depthStencilInfo.setMinDepthBounds(0.0f); // Optional
	configInfo.depthStencilInfo.setMaxDepthBounds(1.0f); // Optional
	configInfo.depthStencilInfo.setStencilTestEnable(VK_FALSE);
	configInfo.depthStencilInfo.setFront({}); // Optional
	configInfo.depthStencilInfo.setBack({}); // Optional

	return configInfo;
}

std::vector<char> VkEnginePipeline::readFile(const std::string& filename)
{

	std::ifstream file{filename, std::ios::ate | std::ios::binary};

	if(!file.is_open())
	{
		throw std::runtime_error("failed to open file: " + filename);
	}

	std::vector<char> buffer(static_cast<size_t>(file.tellg()));

	file.seekg(0);
	file.read(buffer.data(), static_cast<long>(buffer.size()));
	file.close();

	return buffer;
}

void VkEnginePipeline::createGraphicsPipeline(const std::string& vertShader,
											  const std::string& fragShader,
											  const PipelineConfigInfo& configInfo)
{

	assert(configInfo.pipelineLayout != VK_NULL_HANDLE &&
		   "Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
	assert(configInfo.renderPass != VK_NULL_HANDLE &&
		   "Cannot create graphics pipeline: no renderPass provided in configInfo");

	const auto vertShaderCode = readFile(vertShader);
	const auto fragShaderCode = readFile(fragShader);

	createShaderModule(vertShaderCode, &pVertShaderModule);
	createShaderModule(fragShaderCode, &pFragShaderModule);

	std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages{};
	shaderStages[0].sType = vk::StructureType::ePipelineShaderStageCreateInfo;
	shaderStages[0].stage = vk::ShaderStageFlagBits::eVertex;
	shaderStages[0].module = pVertShaderModule;
	shaderStages[0].pName = "main";
	shaderStages[0].pNext = nullptr;

	shaderStages[1].sType = vk::StructureType::ePipelineShaderStageCreateInfo;
	shaderStages[1].stage = vk::ShaderStageFlagBits::eFragment;
	shaderStages[1].module = pFragShaderModule;
	shaderStages[1].pName = "main";
	shaderStages[1].pNext = nullptr;

	constexpr vk::PipelineVertexInputStateCreateInfo vertexInputInfo({}, 0, nullptr, 0, nullptr);

	const vk::PipelineViewportStateCreateInfo viewportInfo(
		{}, 1, &configInfo.viewport, 1, &configInfo.scissor);

	const vk::GraphicsPipelineCreateInfo pipelineInfo({},
													  2,
													  shaderStages.data(),
													  &vertexInputInfo,
													  &configInfo.inputAssemblyInfo,
													  nullptr,
													  &viewportInfo,
													  &configInfo.rasterizationInfo,
													  &configInfo.multisampleInfo,
													  &configInfo.depthStencilInfo,
													  &configInfo.colorBlendInfo,
													  nullptr,
													  configInfo.pipelineLayout,
													  configInfo.renderPass,
													  configInfo.subpass,
													  nullptr,
													  -1);

	if(const vk::Result result = mDevice.device().createGraphicsPipelines(
		   VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pGraphicsPipeline);
	   result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to create graphics pipeline");
	}
}

void VkEnginePipeline::createShaderModule(const std::vector<char>& code,
										  vk::ShaderModule* shaderModule) const
{
	const vk::ShaderModuleCreateInfo createInfo(
		{}, code.size(), reinterpret_cast<const uint32_t*>(code.data()));

	if(const vk::Result result =
		   mDevice.device().createShaderModule(&createInfo, nullptr, shaderModule);
	   result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to create shader module");
	}
}

} // namespace vke