//
// Created by zphrfx on 16/12/2023.
//

#pragma once

#include "engine_device.hpp"
#include "engine_model.hpp"

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
	std::array<VkDynamicState,2> pDynamicStateEnables{};
	VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	u32 subpass = 0;
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

	void bind(const VkCommandBuffer* commandBuffer) const;

   private:
	static char* readFile(const std::string& filename, size_t& bufferSize);

	void createGraphicsPipeline(const std::string& vertShader, const std::string& fragShader,
	                            const PipelineConfigInfo& configInfo);

	void createShaderModule(const char* code, size_t codeSize, VkShaderModule* shaderModule) const;

	const VkEngineDevice& mDevice;
	VkPipeline pGraphicsPipeline = VK_NULL_HANDLE;

	Shader mShaders{};
};
}  // namespace vke