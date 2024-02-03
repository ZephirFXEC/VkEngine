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

	bool isFrameInProgress() const { return isFrameStarted; }
	const std::shared_ptr<VkEngineSwapChain>& getSwapChain() { return mVkSwapChain; }

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
	static void endSwapChainRenderPass(const VkCommandBuffer* commandBuffer);

	void run();

   private:
	void createCommandBuffers();
	void drawFrame();
	void recreateSwapChain();
	void freeCommandBuffers() const;

	VkEngineWindow& mVkWindow;
	VkEngineDevice& mVkDevice;
	std::shared_ptr<VkEngineSwapChain> mVkSwapChain = nullptr;

	std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> mVkCommandBuffers{VK_NULL_HANDLE};

	u32 mCurrentImage = 0;
	u32 mCurrentFrame = 0;

	bool isFrameStarted = false;
};
}  // namespace vke
