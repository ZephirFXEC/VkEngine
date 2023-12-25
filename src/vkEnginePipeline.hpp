//
// Created by zphrfx on 16/12/2023.
//

#pragma once

#include <vector>

#include "vkEngineDevice.hpp"

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
	VkDynamicState* pDynamicStateEnables = nullptr;
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
	static char* readFile(const std::string& filename, size_t& bufferSize);

	void createGraphicsPipeline(const std::string& vertShader, const std::string& fragShader,
	                            const PipelineConfigInfo& configInfo);

	void createShaderModule(const char* code, size_t codeSize, VkShaderModule* shaderModule) const;

	VkEngineDevice& mDevice;
	VkPipeline pGraphicsPipeline = VK_NULL_HANDLE;

	struct Shader {
		VkShaderModule pVertShaderModule = VK_NULL_HANDLE;
		VkShaderModule pFragShaderModule = VK_NULL_HANDLE;
	} mShaders;
};
}  // namespace vke