#pragma once

#include "utils/utility.hpp"
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

	[[nodiscard]] const VkFramebuffer& getFrameBuffer(const uint32_t index) const {
		return ppSwapChainFramebuffers[index];
	}

	NDC_INLINE const VkRenderPass& getRenderPass() const { return pRenderPass; }

	NDC_INLINE const VkImageView& getImageView(const uint32_t index) const { return mSwapChainImages.ppImageViews[index]; }

	NDC_INLINE const VkFormat& getSwapChainImageFormat() const { return mSwapChainImageFormat; }

	NDC_INLINE const VkExtent2D& getSwapChainExtent() const { return mSwapChainExtent; }

	NDC_INLINE uint32_t width() const { return mSwapChainExtent.width; }

	NDC_INLINE uint32_t height() const { return mSwapChainExtent.height; }

	NDC_INLINE size_t imageCount() const { return mSwapChainImageCount; }

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

	void createSyncObjects();

	// Helper functions
	static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

	static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	[[nodiscard]] VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;


	struct SyncPrimitives {
		VkSemaphore* ppImageAvailableSemaphores = nullptr; // Semaphores for image availability
		VkSemaphore* ppRenderFinishedSemaphores = nullptr; // Semaphores for render finishing
		VkFence* ppInFlightFences = nullptr;               // Fences for in-flight operations
		VkFence* ppInFlightImages = nullptr;               // Fences for in-flight images

		~SyncPrimitives() {
			delete[] ppImageAvailableSemaphores;
			delete[] ppRenderFinishedSemaphores;
			delete[] ppInFlightFences;
			delete[] ppInFlightImages;
		}
	};

	struct VkImageRessource {
		VkImageRessource() {
			ppImages = new VkImage[MAX_FRAMES_IN_FLIGHT];
			ppImageViews = new VkImageView[MAX_FRAMES_IN_FLIGHT];
			ppImageMemorys = new VkDeviceMemory[MAX_FRAMES_IN_FLIGHT];
		}

		VkImageRessource(const VkImageRessource&) = delete;
		VkImageRessource& operator=(const VkImageRessource&) = delete;

		VkImage* ppImages = nullptr;
		VkImageView* ppImageViews = nullptr;
		VkDeviceMemory* ppImageMemorys = nullptr;

		~VkImageRessource() {
			delete[] ppImages;
			delete[] ppImageViews;
			delete[] ppImageMemorys;
		}
	};

	VkImageRessource mSwapChainImages{};
	VkImageRessource mDepthImages{};

	SyncPrimitives mSyncPrimitives{};

	VkEngineDevice& mDevice;
	VkRenderPass pRenderPass = VK_NULL_HANDLE;
	VkSwapchainKHR pSwapChain = VK_NULL_HANDLE;

	VkFormat mSwapChainImageFormat{};
	VkExtent2D mSwapChainExtent{};
	VkExtent2D mWindowExtent{};

	// Framebuffers
	VkFramebuffer* ppSwapChainFramebuffers = nullptr;

	// Pointer to old swap chain
	std::shared_ptr<VkEngineSwapChain> pOldSwapChain = nullptr;

	size_t mCurrentFrame = 0;
	size_t mSwapChainImageCount = 0;
};
}  // namespace vke