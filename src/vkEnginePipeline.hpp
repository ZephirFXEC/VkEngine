//
// Created by zphrfx on 16/12/2023.
//

#ifndef VKPIPELINE_HPP
#define VKPIPELINE_HPP


#include "vkEngineDevice.hpp"
#include "vkEngineModel.hpp"

#include <vector>

namespace vke
{

struct PipelineConfigInfo
{
	VkViewport viewport{};
	VkRect2D scissor{};
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
	VkPipelineMultisampleStateCreateInfo multisampleInfo{};
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
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

	void bind(VkCommandBuffer commandBuffer) const;

private:
	static std::vector <char> readFile(const std::string& filename);

	void createGraphicsPipeline(const std::string& vertShader,
	                            const std::string& fragShader,
	                            const PipelineConfigInfo& configInfo);

	void createShaderModule(const std::vector <char>& code, VkShaderModule* shaderModule) const;

	VkEngineDevice& mDevice;
	VkPipeline pGraphicsPipeline = VK_NULL_HANDLE;
	VkShaderModule pVertShaderModule = VK_NULL_HANDLE;
	VkShaderModule pFragShaderModule = VK_NULL_HANDLE;
};
} // namespace vke

#endif // VKPIPELINE_H