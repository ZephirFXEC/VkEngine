//
// Created by zphrfx on 16/12/2023.
//

#ifndef VKPIPELINE_H
#define VKPIPELINE_H

#include <string>
#include <vector>

#include "vkEngineDevice.hpp"

namespace vke {

struct PipelineConfigInfo {

    PipelineConfigInfo() = default;

    VkViewport viewport{};
    VkRect2D scissor{};
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
    VkPipelineMultisampleStateCreateInfo multisampleInfo{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class VkEnginePipeline {
  public:
    VkEnginePipeline(VkEngineDevice &device, const std::string &vertShader,
                     const std::string &fragShader, const PipelineConfigInfo &configInfo);
    ~VkEnginePipeline();

    VkEnginePipeline(const VkEnginePipeline &) = delete;
    void operator=(const VkEnginePipeline &) = delete;

    static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

  private:
    static std::vector<char> readFile(const std::string &filename);

    void createGraphicsPipeline(const std::string &vertShader, const std::string &fragShader,
                                const PipelineConfigInfo &configInfo);

    void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

    VkEngineDevice &mDevice;
    VkPipeline pGraphicsPipeline{};
    VkShaderModule pVertShaderModule{};
    VkShaderModule pFragShaderModule{};
};
} // namespace vke

#endif // VKPIPELINE_H