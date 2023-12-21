//
// Created by zphrfx on 16/12/2023.
//

#ifndef VKPIPELINE_H
#define VKPIPELINE_H

#include <string>
#include <vector>

#include "vkEngineDevice.hpp"

namespace vke
{

struct PipelineConfigInfo
{
	vk::Viewport viewport{};
	vk::Rect2D scissor{};
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	vk::PipelineRasterizationStateCreateInfo rasterizationInfo{};
	vk::PipelineMultisampleStateCreateInfo multisampleInfo{};
	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
	vk::PipelineColorBlendStateCreateInfo colorBlendInfo{};
	vk::PipelineDepthStencilStateCreateInfo depthStencilInfo{};
	vk::RenderPass renderPass{};
	vk::PipelineLayout pipelineLayout{};
	uint32_t subpass = 0;
};

class VkEnginePipeline
{
public:
	VkEnginePipeline(VkEngineDevice& device,
					 const std::string& vertShader,
					 const std::string& fragShader,
					 const PipelineConfigInfo& configInfo);
	~VkEnginePipeline();

	VkEnginePipeline(const VkEnginePipeline&) = delete;
	void operator=(const VkEnginePipeline&) = delete;

	static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);
	void bind(vk::CommandBuffer commandBuffer) const;

	[[nodiscard]] const vk::Pipeline& getPipeline() const
	{
		return pGraphicsPipeline;
	}

private:
	static std::vector<char> readFile(const std::string& filename);

	void createGraphicsPipeline(const std::string& vertShader,
								const std::string& fragShader,
								const PipelineConfigInfo& configInfo);

	void createShaderModule(const std::vector<char>& code, vk::ShaderModule* shaderModule) const;

	VkEngineDevice& mDevice;
	vk::Pipeline pGraphicsPipeline{};
	vk::ShaderModule pVertShaderModule{};
	vk::ShaderModule pFragShaderModule{};
};
} // namespace vke

#endif // VKPIPELINE_H