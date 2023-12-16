//
// Created by zphrfx on 16/12/2023.
//

#include "vkEnginePipeline.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace vke {
VkEnginePipeline::VkEnginePipeline(VkEngineDevice &device,
                                   const std::string &vertShader,
                                   const std::string &fragShader,
                                   const PipelineConfigInfo &configInfo)
    : mDevice(device) {

    createGraphicsPipeline(vertShader, fragShader, configInfo);
}
VkEnginePipeline::~VkEnginePipeline() = default;

PipelineConfigInfo
VkEnginePipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {

    constexpr PipelineConfigInfo configInfo{};

    return configInfo;
}

std::vector<char> VkEnginePipeline::readFile(const std::string &filename) {

    std::ifstream file{filename, std::ios::ate | std::ios::binary};

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!" + filename);
    }

    const size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

void VkEnginePipeline::createGraphicsPipeline(
    const std::string &vertShader, const std::string &fragShader,
    const PipelineConfigInfo &configInfo) {

    const auto vertShaderCode = readFile(vertShader);
    const auto fragShaderCode = readFile(fragShader);

    std::cout << "vertShaderCode: " << vertShaderCode.size() << '\n';
    std::cout << "fragShaderCode: " << fragShaderCode.size() << '\n';
}
void VkEnginePipeline::createShaderModule(const std::vector<char> &code,
                                          VkShaderModule *shaderModule) {
    VkShaderModuleCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t *>(code.data())};

    if (vkCreateShaderModule(mDevice.device(), &createInfo, nullptr,
                             shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Shader Module");
    }
}
} // namespace vke