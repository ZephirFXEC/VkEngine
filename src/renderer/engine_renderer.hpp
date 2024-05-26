//
// Created by zphrfx on 03/02/2024.
//

#pragma once


#include <cassert>

#include "core/engine_device.hpp"
#include "core/engine_swapchain.hpp"
#include "core/engine_window.hpp"

namespace vke {
class VkEngineRenderer {
   public:
	VkEngineRenderer(const VkEngineDevice& device, VkEngineWindow& window);
	~VkEngineRenderer();

	VkEngineRenderer(const VkEngineRenderer&) = delete;
	VkEngineRenderer& operator=(const VkEngineRenderer&) = delete;

	bool isFrameInProgress() const { return isFrameStarted; }
	float getAspectRatio() const { return mVkSwapChain->extentAspectRatio(); }
	VkRenderPass getSwapChainRenderPass() const { return mVkSwapChain->getRenderPass(); }

	VkCommandBuffer getCurrentCommandBuffer() const {
		assert(isFrameStarted && "Cannot get command buffer when frame not in progress.");
		return mVkCommandBuffers[mCurrentFrame];
	}

	u32 getFrameIndex() const {
		assert(isFrameStarted && "Cannot get frame index when frame not in progress.");
		return mCurrentFrame;
	}

	VkCommandBuffer beginFrame();
	void endFrame();
	void beginSwapChainRenderPass(const VkCommandBuffer* commandBuffer) const;
	static void endSwapChainRenderPass(const VkCommandBuffer* commandBuffer) ;

	void run();

   private:
	void createCommandBuffers();
	void drawFrame();
	void recreateSwapChain();
	void freeCommandBuffers() const;

	const VkEngineDevice& mVkDevice;
	VkEngineWindow& mVkWindow;

	std::unique_ptr<VkEngineSwapChain> mVkSwapChain;
	std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> mVkCommandBuffers{VK_NULL_HANDLE};

	u32 mCurrentImage = 0;
	u32 mCurrentFrame = 0;

	bool isFrameStarted = false;
};
}  // namespace vke
