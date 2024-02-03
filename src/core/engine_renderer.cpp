//
// Created by zphrfx on 03/02/2024.
//

#include "engine_renderer.hpp"

#include "utils/logger.hpp"
#include "utils/memory.hpp"
#include "utils/types.hpp"

namespace vke {
VkEngineRenderer::VkEngineRenderer(VkEngineWindow& window, VkEngineDevice& device)
    : mVkWindow(window), mVkDevice(device) {
	recreateSwapChain();
	createCommandBuffers();
}

VkEngineRenderer::~VkEngineRenderer() { freeCommandBuffers(); }

void VkEngineRenderer::createCommandBuffers() {
	mCommandBuffer = {
	    .ppVkCommandBuffers = Memory::allocMemory<VkCommandBuffer>(mVkSwapChain->getImageCount(), MEMORY_TAG_VULKAN),
	    .mSize = mVkSwapChain->getImageCount()};

	const VkCommandBufferAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	                                            .commandPool = mVkSwapChain->getCommandPool(),
	                                            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	                                            .commandBufferCount = mCommandBuffer.mSize};


	VK_CHECK(vkAllocateCommandBuffers(mVkDevice.getDevice(), &allocInfo, mCommandBuffer.ppVkCommandBuffers));
}


void VkEngineRenderer::freeCommandBuffers() const {
	vkResetCommandPool(mVkDevice.getDevice(), mVkSwapChain->getCommandPool(), 0);
	Memory::freeMemory(mCommandBuffer.ppVkCommandBuffers, mCommandBuffer.mSize, MEMORY_TAG_VULKAN);
}

void VkEngineRenderer::recreateSwapChain() {
	auto extent = mVkWindow.getExtent();

	while (extent.width == 0 || extent.height == 0) {
		extent = mVkWindow.getExtent();
		glfwWaitEvents();
	}

	VK_CHECK(vkDeviceWaitIdle(mVkDevice.getDevice()));

	if (mVkSwapChain == nullptr) {
		mVkSwapChain = std::make_unique<VkEngineSwapChain>(mVkDevice, extent);
	} else {
		mVkSwapChain = std::make_unique<VkEngineSwapChain>(mVkDevice, extent, std::move(mVkSwapChain));

		if (mVkSwapChain->getImageCount() != mCommandBuffer.mSize) {
			freeCommandBuffers();
			createCommandBuffers();
		}
	}

	// TODO: create pipeline
}

VkCommandBuffer VkEngineRenderer::beginFrame() {
	assert(!isFrameStarted && "Can't call beginFrame while frame is in progress.");

	const VkResult result = mVkSwapChain->acquireNextImage(&mCurrentFrame);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return VK_NULL_HANDLE;
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	isFrameStarted = true;

	auto* const commandBuffer = getCurrentCommandBuffer();
	constexpr VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin recording command buffer!");
	}

	return commandBuffer;
}

void VkEngineRenderer::endFrame() {
	assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
	auto* const commandBuffer = getCurrentCommandBuffer();
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}


	if (const VkResult result = mVkSwapChain->submitCommandBuffers(&commandBuffer, &mCurrentFrame);
	    result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mVkWindow.wasWindowResized()) {
		mVkWindow.resetWindowResizedFlag();
		recreateSwapChain();

	} else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}


	isFrameStarted = false;
	mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}


void VkEngineRenderer::beginSwapChainRenderPass(const VkCommandBuffer commandBuffer) const {
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {{0.0f, 0.0f, 0.0f, 0.0f}};
	clearValues[1].depthStencil = {1.0f, 0};

	const VkRenderPassBeginInfo renderPassInfo{
	    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
	    .renderPass = mVkSwapChain->getRenderPass(),
	    .framebuffer = mVkSwapChain->getFrameBuffer(static_cast<u32>(mCurrentFrame)),
	    .renderArea = {.offset = {0, 0}, .extent = mVkSwapChain->getSwapChainExtent()},
	    .clearValueCount = static_cast<u32>(clearValues.size()),
	    .pClearValues = clearValues.data(),
	};

	const VkViewport viewport{
	    .x = 0.0f,
	    .y = 0.0f,
	    .width = static_cast<float>(mVkSwapChain->getSwapChainExtent().width),
	    .height = static_cast<float>(mVkSwapChain->getSwapChainExtent().height),
	    .minDepth = 0.0f,
	    .maxDepth = 1.0f,
	};

	const VkRect2D scissor{
	    .offset = {0, 0},
	    .extent = mVkSwapChain->getSwapChainExtent(),
	};

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}
void VkEngineRenderer::endSwapChainRenderPass(const VkCommandBuffer commandBuffer) const {
	assert(isFrameStarted && "Cannot end render pass when frame is not in progress.");

	assert(commandBuffer == mCommandBuffer.ppVkCommandBuffers[mCurrentFrame] &&
	       "Can only end the render pass on the currently active command buffer!");

	vkCmdEndRenderPass(commandBuffer);
}


}  // namespace vke