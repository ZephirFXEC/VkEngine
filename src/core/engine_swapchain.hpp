#pragma once

#include <vk_mem_alloc.h>

#include <memory>

#include "engine_device.hpp"
#include "utils/types.hpp"

namespace vke {
class VkEngineSwapChain {
   public:
	explicit VkEngineSwapChain() = delete;

	VkEngineSwapChain(const VkEngineDevice& deviceRef, VkExtent2D windowExtent);

	VkEngineSwapChain(const VkEngineDevice& deviceRef, VkExtent2D windowExtent,
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

	[[nodiscard]] const VkEngineDevice& getEngineDevice() const { return mDevice; }

	[[nodiscard]] const VkCommandPool& getCommandPool() const { return mFrameData.at(mCurrentFrame).pCommandPool; }

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

	void createCommandPools();

	void createSyncObjects();

	// Helper functions
	static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	[[nodiscard]] VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;


	void copyBufferToImage(const VkBuffer* buffer, const VkImage* image, u32 width, u32 height, u32 layerCount);

	void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkImage& image, VmaAllocation& imageMemory) const;

	const VkEngineDevice& mDevice;

	VkRenderPass pRenderPass = VK_NULL_HANDLE;
	VkSwapchainKHR pSwapChain{};

	VkImageRessource mSwapChainImages{};
	VkImageRessource mDepthImages{};
	SyncPrimitives mSyncPrimitives{};
	VkFormat mSwapChainImageFormat{};
	VkFormat mDepthFormat{};
	VkExtent2D mSwapChainExtent{};
	VkExtent2D mWindowExtent{};

	VkFramebuffer* ppSwapChainFramebuffers = nullptr;
	std::array<FrameData, MAX_FRAMES_IN_FLIGHT> mFrameData{};
	std::shared_ptr<VkEngineSwapChain> pOldSwapChain = nullptr;

	u32 mSwapChainImageCount = 0;
	u32 mCurrentFrame = 0;
};
}  // namespace vke
