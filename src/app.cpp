#include "app.hpp"

#include <stdexcept>

namespace vke {
App::App() {
    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
}

App::~App() {
    vkDestroyPipelineLayout(mVkDevice.device(), pVkPipelineLayout, nullptr);
}

void App::run() {
    while (!mVkWindow.shouldClose()) { // while window is open
        glfwPollEvents();              // poll for events
        drawFrame();                   // draw frame
    }

    vkDeviceWaitIdle(mVkDevice.device());
}

void App::createPipelineLayout() {
    constexpr VkPipelineLayoutCreateInfo pipelineLayoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr};

    if (vkCreatePipelineLayout(mVkDevice.device(), &pipelineLayoutInfo, nullptr,
                               &pVkPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void App::createPipeline() {

    auto pipelineConfig =
        VkEnginePipeline::defaultPipelineConfigInfo(mVkSwapChain.width(), mVkSwapChain.height());

    pipelineConfig.renderPass = mVkSwapChain.getRenderPass();
    pipelineConfig.pipelineLayout = pVkPipelineLayout;

    pVkPipeline =
        std::make_unique<VkEnginePipeline>(mVkDevice, "../shaders/simple_shader.vert.spv",
                                           "../shaders/simple_shader.frag.spv", pipelineConfig);
}

void App::createCommandBuffers() {
    ppVkCommandBuffers.resize(mVkSwapChain.imageCount());

    const VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = mVkDevice.getCommandPool(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(mVkSwapChain.imageCount())
    };

    if (vkAllocateCommandBuffers(mVkDevice.device(), &allocInfo, ppVkCommandBuffers.data()) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create command buffers!");
    }

    for (uint32_t i = 0; i < mVkSwapChain.imageCount(); ++i) {
         VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        };

        if (vkBeginCommandBuffer(ppVkCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = mVkSwapChain.getRenderPass(),
            .framebuffer = mVkSwapChain.getFrameBuffer(i),
            .renderArea = {.offset = {0, 0},.extent = mVkSwapChain.getSwapChainExtent()},
        };

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(ppVkCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        pVkPipeline->bind(ppVkCommandBuffers[i]);
        vkCmdDraw(ppVkCommandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(ppVkCommandBuffers[i]);
        if (vkEndCommandBuffer(ppVkCommandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void App::drawFrame() {
    uint32_t imageIndex = 0;
    auto result = mVkSwapChain.acquireNextImage(&imageIndex);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    result = mVkSwapChain.submitCommandBuffers(&ppVkCommandBuffers[imageIndex], &imageIndex);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

} // namespace vke