#pragma once

#include <memory>

#include "engine_device.hpp"
#include "utils/types.hpp"

namespace vke {
class VkEngineSwapChain {
   public:
	explicit VkEngineSwapChain() = delete;

	VkEngineSwapChain(std::shared_ptr<VkEngineDevice> device, VkExtent2D windowExtent);

	VkEngineSwapChain(std::shared_ptr<VkEngineDevice> device, VkExtent2D windowExtent,
	                  const std::shared_ptr<VkEngineSwapChain>& previous);

	~VkEngineSwapChain();

	VkEngineSwapChain(const VkEngineSwapChain&) = delete;

	VkEngineSwapChain& operator=(const VkEngineSwapChain&) = delete;

	[[nodiscard]] const VkFramebuffer& getFrameBuffer(const u32 index) const { return ppSwapChainFramebuffers[index]; }

	[[nodiscard]] const VkImageView& getImageView(const u32 index) const {
		return mSwapChainImages.ppImageViews[index];
	}

	[[nodiscard]] const VkSwapchainKHR& getSwapChain() const { return pSwapChain; }
	[[nodiscard]] const VkRenderPass& getRenderPass() const { return pRenderPass; }
	[[nodiscard]] std::shared_ptr<VkEngineDevice> getEngineDevice() const { return mDevice; }
	[[nodiscard]] const VkFormat& getSwapChainImageFormat() const { return mSwapChainImageFormat; }
	[[nodiscard]] const VkExtent2D& getSwapChainExtent() const { return mSwapChainExtent; }
	[[nodiscard]] u32 getWidth() const { return mSwapChainExtent.width; }
	[[nodiscard]] u32 getHeight() const { return mSwapChainExtent.height; }
	[[nodiscard]] u32 getImageCount() const { return mSwapChainImageCount; }
	[[nodiscard]] VkFormat findDepthFormat() const;

	VkResult acquireNextImage(u32* imageIndex) const;

	VkResult submitCommandBuffers(const VkCommandBuffer* buffers, const u32* imageIndex);

	bool compareSwapFormats(const VkEngineSwapChain& other) const {
		return other.mSwapChainImageFormat == mSwapChainImageFormat && other.mDepthFormat == mDepthFormat;
	}

	[[nodiscard]] float extentAspectRatio() const {
		return static_cast<float>(mSwapChainExtent.width) / static_cast<float>(mSwapChainExtent.height);
	}


   private:
	void init();

	void createSwapChain();

	void createImageViews();

	void createDepthResources();

	void createRenderPass();

	void createFramebuffers();

	void createSyncObjects();

	// Helper functions
	static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	[[nodiscard]] VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

	std::shared_ptr<VkEngineDevice> mDevice{};
	DeletionQueue mDeletionQueue{};

	VkRenderPass pRenderPass = VK_NULL_HANDLE;
	VkSwapchainKHR pSwapChain = VK_NULL_HANDLE;

	VkImageRessource mSwapChainImages{};
	VkImageRessource mDepthImages{};
	SyncPrimitives mSyncPrimitives{};
	VkFormat mSwapChainImageFormat{};
	VkFormat mDepthFormat{};
	VkExtent2D mSwapChainExtent{};
	VkExtent2D mWindowExtent{};

	VkFramebuffer* ppSwapChainFramebuffers = nullptr;
	std::shared_ptr<VkEngineSwapChain> pOldSwapChain = nullptr;

	u32 mSwapChainImageCount = 0;
	u32 mCurrentFrame = 0;
};
}  // namespace vke
