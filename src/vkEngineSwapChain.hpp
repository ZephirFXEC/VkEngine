#ifndef VKENGINE_SWAPCHAIN_HPP
#define VKENGINE_SWAPCHAIN_HPP

#include "vkEngineDevice.hpp"

// vulkan headers
#include <vulkan/vulkan.hpp>


namespace vke {

class VkEngineSwapChain {
  public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    VkEngineSwapChain(VkEngineDevice &deviceRef, VkExtent2D windowExtent);
    VkEngineSwapChain(VkEngineDevice &deviceRef, VkExtent2D windowExtent,
                      std::shared_ptr<VkEngineSwapChain> previous);
    ~VkEngineSwapChain();

    VkEngineSwapChain(const VkEngineSwapChain &) = delete;
    void operator=(const VkEngineSwapChain &) = delete;

    const VkFramebuffer &getFrameBuffer(const size_t index) const { return ppSwapChainFramebuffers[index]; }
    const VkRenderPass &getRenderPass() const { return pRenderPass; }
    const VkImageView &getImageView(const size_t index) const { return ppSwapChainImageViews[index]; }
    const VkFormat &getSwapChainImageFormat() const { return mSwapChainImageFormat; }
    const VkExtent2D &getSwapChainExtent() const { return mSwapChainExtent; }

    uint32_t width() const { return mSwapChainExtent.width; }
    uint32_t height() const { return mSwapChainExtent.height; }
    size_t imageCount() const { return sizeof(ppSwapChainImages) / sizeof(VkImage); }

    float extentAspectRatio() const {
        return static_cast<float>(mSwapChainExtent.width) /
               static_cast<float>(mSwapChainExtent.height);
    }

    VkFormat findDepthFormat() const;
    VkResult acquireNextImage(uint32_t *imageIndex) const;
    VkResult submitCommandBuffers(const VkCommandBuffer *buffers, const uint32_t *imageIndex);

  private:
    void init();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    // Helper functions
    static VkSurfaceFormatKHR
    chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    static VkPresentModeKHR
    chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) const;

    VkFormat mSwapChainImageFormat{};
    VkExtent2D mSwapChainExtent{};

    VkFramebuffer* ppSwapChainFramebuffers = VK_NULL_HANDLE;
    VkRenderPass pRenderPass = VK_NULL_HANDLE;

    VkImage* ppDepthImages = VK_NULL_HANDLE;
    VkDeviceMemory* ppDepthImageMemorys = VK_NULL_HANDLE;
    VkImageView* ppDepthImageViews = VK_NULL_HANDLE;
    VkImage* ppSwapChainImages = VK_NULL_HANDLE;
    VkImageView* ppSwapChainImageViews = VK_NULL_HANDLE;

    VkEngineDevice &device;
    VkExtent2D windowExtent{};

    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::shared_ptr<VkEngineSwapChain> oldSwapChain = nullptr;

    VkSemaphore* ppImageAvailableSemaphores = VK_NULL_HANDLE;
    VkSemaphore* ppRenderFinishedSemaphores = VK_NULL_HANDLE;
    VkFence* ppInFlightFences = VK_NULL_HANDLE;
    VkFence* ppImagesInFlight = VK_NULL_HANDLE;
    size_t currentFrame = 0;
};

} // namespace vke

#endif // !VKENGINE_SWAPCHAIN_HPP