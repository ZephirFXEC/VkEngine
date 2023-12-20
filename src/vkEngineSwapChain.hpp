#ifndef VKENGINE_SWAPCHAIN_HPP
#define VKENGINE_SWAPCHAIN_HPP

#include "vkEngineDevice.hpp"

// vulkan headers
#include <vulkan/vulkan.hpp>

// std lib headers
#include <vector>

namespace vke {

class VkEngineSwapChain {
  public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    VkEngineSwapChain(VkEngineDevice &deviceRef, VkExtent2D windowExtent);
    VkEngineSwapChain(VkEngineDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<VkEngineSwapChain> previous);
    ~VkEngineSwapChain();

    VkEngineSwapChain(const VkEngineSwapChain &) = delete;
    void operator=(const VkEngineSwapChain &) = delete;

    [[nodiscard]] const vk::Framebuffer &getFrameBuffer(const uint32_t index) const { return ppSwapChainFramebuffers[index]; }
    [[nodiscard]] const vk::RenderPass &getRenderPass() const { return pRenderPass; }
    [[nodiscard]] const vk::ImageView &getImageView(const uint32_t index) const { return ppSwapChainImageViews[index]; }
    [[nodiscard]] const vk::Format &getSwapChainImageFormat() const { return mSwapChainImageFormat; }
    [[nodiscard]] const vk::Extent2D &getSwapChainExtent() const { return mSwapChainExtent; }

    [[nodiscard]] uint32_t width() const { return mSwapChainExtent.width; }
    [[nodiscard]] uint32_t height() const { return mSwapChainExtent.height; }
    [[nodiscard]] size_t imageCount() const { return swapChainImages.size(); }

    [[nodiscard]] float extentAspectRatio() const {
        return static_cast<float>(mSwapChainExtent.width) /
               static_cast<float>(mSwapChainExtent.height);
    }

    [[nodiscard]] VkFormat findDepthFormat() const;
    vk::Result acquireNextImage(uint32_t *imageIndex) const;
    vk::Result submitCommandBuffers(const vk::CommandBuffer *buffers, const uint32_t *imageIndex);

  private:
    void init();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    // Helper functions
    static vk::SurfaceFormatKHR
    chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
    static vk::PresentModeKHR
    chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
    [[nodiscard]] vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) const;

    vk::Format mSwapChainImageFormat{};
    vk::Extent2D mSwapChainExtent{};

    vk::Framebuffer* ppSwapChainFramebuffers = VK_NULL_HANDLE;
    vk::RenderPass pRenderPass = VK_NULL_HANDLE;

    std::vector<vk::Image> swapChainImages{};
    std::vector<vk::Image> depthImages{};

    vk::DeviceMemory* ppDepthImageMemorys = VK_NULL_HANDLE;
    vk::ImageView* ppDepthImageViews = VK_NULL_HANDLE;
    vk::ImageView* ppSwapChainImageViews = VK_NULL_HANDLE;

    VkEngineDevice &device;
    vk::Extent2D windowExtent{};

    vk::SwapchainKHR swapChain = VK_NULL_HANDLE;
    std::shared_ptr<VkEngineSwapChain> oldSwapChain = nullptr;

    vk::Semaphore* ppImageAvailableSemaphores = VK_NULL_HANDLE;
    vk::Semaphore* ppRenderFinishedSemaphores = VK_NULL_HANDLE;
    vk::Fence* ppInFlightFences = VK_NULL_HANDLE;
    vk::Fence* ppImagesInFlight = VK_NULL_HANDLE; // no delete[] idk why but it works

    size_t currentFrame = 0;
};

} // namespace vke

#endif // !VKENGINE_SWAPCHAIN_HPP