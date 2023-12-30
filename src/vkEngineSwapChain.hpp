#pragma once

#include "utils/utility.hpp"
#include "vkEngineDevice.hpp"

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

	NDC_INLINE const VkFramebuffer& getFrameBuffer(const uint32_t index) const {
		return ppSwapChainFramebuffers[index];
	}

	NDC_INLINE const VkImageView& getImageView(const uint32_t index) const {
		return mSwapChainImages.ppImageViews[index];
	}

	// TYPE				NAME			VARIABLE //
	GETTERS(VkSwapchainKHR, SwapChain, pSwapChain)
	GETTERS(VkRenderPass, RenderPass, pRenderPass)
	GETTERS(VkEngineDevice, EngineDevice, mDevice)
	GETTERS(VkCommandPool, CommandPool, mFrameData[mCurrentFrame].pCommandPool)
	GETTERS(VkFormat, SwapChainImageFormat, mSwapChainImageFormat)
	GETTERS(VkExtent2D, SwapChainExtent, mSwapChainExtent)
	GETTERS(uint32_t, Width, mSwapChainExtent.width)
	GETTERS(uint32_t, Height, mSwapChainExtent.height)
	GETTERS(uint32_t, ImageCount, mSwapChainImageCount)

	NDC_INLINE float extentAspectRatio() const {
		return static_cast<float>(mSwapChainExtent.width) / static_cast<float>(mSwapChainExtent.height);
	}

	[[nodiscard]] VkFormat findDepthFormat() const;

	VkResult acquireNextImage(uint32_t* imageIndex) const;

	VkResult submitCommandBuffers(const VkCommandBuffer* buffers, const uint32_t* imageIndex);

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

	// Buffer Helper Functions
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
	                  Alloc& bufferMemory) const;

	void copyBufferToImage(const VkBuffer* buffer, const VkImage* image, uint32_t width, uint32_t height,
	                       uint32_t layerCount);

	void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image,
	                         Alloc& imageMemory) const;

	const VkEngineDevice& mDevice;

	VkRenderPass pRenderPass = VK_NULL_HANDLE;
	VkSwapchainKHR pSwapChain = VK_NULL_HANDLE;

	VkImageRessource mSwapChainImages{};
	VkImageRessource mDepthImages{};
	SyncPrimitives mSyncPrimitives{};
	VkFormat mSwapChainImageFormat{};
	VkExtent2D mSwapChainExtent{};
	VkExtent2D mWindowExtent{};

	DeletionQueue mMainDeletionQueue{};

	VkFramebuffer* ppSwapChainFramebuffers = nullptr;
	std::array<FrameData, MAX_FRAMES_IN_FLIGHT> mFrameData{};
	std::shared_ptr<VkEngineSwapChain> pOldSwapChain = nullptr;

	uint32_t mSwapChainImageCount = 0;
	uint32_t mCurrentFrame = 0;
};
}  // namespace vke