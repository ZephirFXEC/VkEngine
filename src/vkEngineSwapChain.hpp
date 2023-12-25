#ifndef VKENGINE_SWAPCHAIN_HPP
#define VKENGINE_SWAPCHAIN_HPP

#include "vkEngineDevice.hpp"

// std lib headers
#include <vector>

namespace vke {
class VkEngineSwapChain {
   public:
	static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	explicit VkEngineSwapChain() = delete;

	VkEngineSwapChain(VkEngineDevice& deviceRef, VkExtent2D windowExtent);

	VkEngineSwapChain(VkEngineDevice& deviceRef, VkExtent2D windowExtent,
	                  const std::shared_ptr<VkEngineSwapChain>& previous);

	~VkEngineSwapChain();

	VkEngineSwapChain(const VkEngineSwapChain&) = delete;

	VkEngineSwapChain& operator=(const VkEngineSwapChain&) = delete;

	[[nodiscard]] const VkFramebuffer& getFrameBuffer(
	    const uint32_t index) const {
		return ppSwapChainFramebuffers[index];
	}

	[[nodiscard]] const VkRenderPass& getRenderPass() const {
		return pRenderPass;
	}

	[[nodiscard]] const VkImageView& getImageView(const uint32_t index) const {
		return ppSwapChainImageViews[index];
	}

	[[nodiscard]] const VkFormat& getSwapChainImageFormat() const {
		return mSwapChainImageFormat;
	}

	[[nodiscard]] const VkExtent2D& getSwapChainExtent() const {
		return mSwapChainExtent;
	}

	[[nodiscard]] uint32_t width() const { return mSwapChainExtent.width; }

	[[nodiscard]] uint32_t height() const { return mSwapChainExtent.height; }

	[[nodiscard]] size_t imageCount() const { return swapChainImages.size(); }

	[[nodiscard]] float extentAspectRatio() const {
		return static_cast<float>(mSwapChainExtent.width) /
		       static_cast<float>(mSwapChainExtent.height);
	}

	[[nodiscard]] VkFormat findDepthFormat() const;

	VkResult acquireNextImage(uint32_t* imageIndex) const;

	VkResult submitCommandBuffers(const VkCommandBuffer* buffers,
	                              const uint32_t* imageIndex);

   private:
	void init();

	void createSwapChain();

	void createImageViews();

	void createDepthResources();

	void createRenderPass();

	void createFramebuffers();

	void createSyncObjects();

	// Helper functions
	static VkSurfaceFormatKHR chooseSwapSurfaceFormat(
	    const std::vector<VkSurfaceFormatKHR>& availableFormats);

	static VkPresentModeKHR chooseSwapPresentMode(
	    const std::vector<VkPresentModeKHR>& availablePresentModes);

	[[nodiscard]] VkExtent2D chooseSwapExtent(
	    const VkSurfaceCapabilitiesKHR& capabilities) const;

	VkEngineDevice& device;
	VkRenderPass pRenderPass = VK_NULL_HANDLE;
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;

	VkFormat mSwapChainImageFormat{};
	VkExtent2D mSwapChainExtent{};
	VkExtent2D windowExtent{};

	std::shared_ptr<VkEngineSwapChain> pOldSwapChain = nullptr;

	std::vector<VkFramebuffer> ppSwapChainFramebuffers{};
	std::vector<VkImage> swapChainImages{};
	std::vector<VkImage> depthImages{};
	std::vector<VkDeviceMemory> ppDepthImageMemorys{};
	std::vector<VkImageView> ppDepthImageViews{};
	std::vector<VkImageView> ppSwapChainImageViews{};
	std::vector<VkSemaphore> ppImageAvailableSemaphores{};
	std::vector<VkSemaphore> ppRenderFinishedSemaphores{};
	std::vector<VkFence> ppInFlightFences{};
	std::vector<VkFence> ppInFlightImages{};

	size_t currentFrame = 0;

	/*
	std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> ppImageAvailableSemaphores{};
	std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> ppRenderFinishedSemaphores{};
	std::array<VkFence, MAX_FRAMES_IN_FLIGHT> ppInFlightFences{};
	std::array<VkFence, MAX_FRAMES_IN_FLIGHT> ppImagesInFlight{};
	*/
};
}  // namespace vke

#endif  // !VKENGINE_SWAPCHAIN_HPP
