//
// Created by zphrfx on 16/12/2023.
//

#ifndef VKPIPELINE_HPP
#define VKPIPELINE_HPP

#include <vector>

#include "vkEngineDevice.hpp"
#include "vkEngineModel.hpp"

namespace vke {
struct PipelineConfigInfo {
	PipelineConfigInfo() = default;

	PipelineConfigInfo(const PipelineConfigInfo&) = delete;

	PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

	VkPipelineViewportStateCreateInfo viewportInfo{};
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
	VkPipelineMultisampleStateCreateInfo multisampleInfo{};
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
	std::vector<VkDynamicState> dynamicStateEnables{};
	VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	uint32_t subpass = 0;
};

class VkEnginePipeline {
   public:
	explicit VkEnginePipeline() = delete;

	VkEnginePipeline(VkEngineDevice& device, const std::string& vertShader, const std::string& fragShader,
	                 const PipelineConfigInfo& configInfo);

	~VkEnginePipeline();

	VkEnginePipeline(const VkEnginePipeline&) = delete;

	VkEnginePipeline& operator=(const VkEnginePipeline&) = delete;

	static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

	void bind(VkCommandBuffer commandBuffer) const;

   private:
	static std::vector<char> readFile(const std::string& filename);

	void createGraphicsPipeline(const std::string& vertShader, const std::string& fragShader,
	                            const PipelineConfigInfo& configInfo);

	void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) const;

	VkEngineDevice& mDevice;
	VkPipeline pGraphicsPipeline = VK_NULL_HANDLE;
	VkShaderModule pVertShaderModule = VK_NULL_HANDLE;
	VkShaderModule pFragShaderModule = VK_NULL_HANDLE;
};
}  // namespace vke

#endif  // VKPIPELINE_HPP