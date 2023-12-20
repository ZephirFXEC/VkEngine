#include "app.hpp"

#include <stdexcept>

namespace vke {
App::App() {
    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
}

App::~App() { vkDestroyPipelineLayout(mVkDevice.device(), mVkPipelineLayout, nullptr); }

void App::run() const {
    while (!mVkWindow.shouldClose()) { // while window is open
        glfwPollEvents();              // poll for events
    }
}

void App::createPipelineLayout() {
    constexpr VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr};

    if (vkCreatePipelineLayout(mVkDevice.device(), &pipelineLayoutInfo, nullptr,
                               &mVkPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void App::createPipeline() {

    auto pipelineConfig =
        VkEnginePipeline::defaultPipelineConfigInfo(mVkSwapChain.width(), mVkSwapChain.height());

    pipelineConfig.renderPass = mVkSwapChain.getRenderPass();
    pipelineConfig.pipelineLayout = mVkPipelineLayout;

    mVkPipeline =
        std::make_unique<VkEnginePipeline>(mVkDevice, "../shaders/simple_shader.vert.spv",
                                           "../shaders/simple_shader.frag.spv", pipelineConfig);
}

void App::createCommandBuffers() {}

void App::drawFrame() {}

} // namespace vke