//
// Created by zphrfx on 03/02/2024.
//

#pragma once


#include <cassert>

#include "engine_device.hpp"
#include "engine_swapchain.hpp"
#include "engine_window.hpp"

namespace vke {
class VkEngineRenderer {
   public:
	VkEngineRenderer(VkEngineWindow& window, VkEngineDevice& device);
	~VkEngineRenderer();

	VkEngineRenderer(const VkEngineRenderer&) = delete;
	VkEngineRenderer& operator=(const VkEngineRenderer&) = delete;

	VkRenderPass getRenderPass() const& { return mVkSwapChain->getRenderPass(); }
	bool isFrameInProgress() const { return isFrameStarted; }
	const std::shared_ptr<VkEngineSwapChain>& getSwapChain() { return mVkSwapChain; }

	VkCommandBuffer getCurrentCommandBuffer() const {
		assert(isFrameStarted && "Cannot get command buffer when frame not in progress.");
		return mCommandBuffer.ppVkCommandBuffers[mCurrentFrame];
	}

	VkCommandBuffer beginFrame();
	void endFrame();
	void beginSwapChainRenderPass(VkCommandBuffer commandBuffer) const;
	void endSwapChainRenderPass(VkCommandBuffer commandBuffer) const;

	void run();

   private:
	void createCommandBuffers();
	void drawFrame();
	void recreateSwapChain();
	void freeCommandBuffers() const;

	VkEngineWindow& mVkWindow;
	VkEngineDevice& mVkDevice;
	std::shared_ptr<VkEngineSwapChain> mVkSwapChain = nullptr;

	struct CommandBuffer {
		VkCommandBuffer* ppVkCommandBuffers{};
		u32 mSize{};  // number of command buffers (could be uint8_t)
	} mCommandBuffer{};

	u32 mCurrentFrame = 0;
	bool isFrameStarted = false;
};
}  // namespace vke
