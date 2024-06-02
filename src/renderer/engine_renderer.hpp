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

	void run();
	void endFrame();
	VkCommandBuffer beginFrame();
	bool isFrameInProgress() const { return isFrameStarted; }
	void beginSwapChainRenderPass(const VkCommandBuffer* commandBuffer) const;
	void endSwapChainRenderPass(const VkCommandBuffer* commandBuffer) const;


	u32 getFrameIndex() const;
	float getAspectRatio() const { return mVkSwapChain->extentAspectRatio(); }
	VkRenderPass getSwapChainRenderPass() const { return mVkSwapChain->getRenderPass(); }
	VkCommandBuffer getCurrentCommandBuffer() const;

   private:
	void drawFrame();
	void recreateSwapChain();
	void createCommandBuffers();
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
