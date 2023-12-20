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

    const VkFramebuffer &getFrameBuffer(int index) const { return swapChainFramebuffers[index]; }
    const VkRenderPass &getRenderPass() const { return renderPass; }
    const VkImageView &getImageView(int index) const { return swapChainImageViews[index]; }
    const VkFormat &getSwapChainImageFormat() const { return swapChainImageFormat; }
    const VkExtent2D &getSwapChainExtent() const { return swapChainExtent; }

    uint32_t width() const { return swapChainExtent.width; }
    uint32_t height() const { return swapChainExtent.height; }
    size_t imageCount() const { return swapChainImages.size(); }

    float extentAspectRatio() const {
        return static_cast<float>(swapChainExtent.width) /
               static_cast<float>(swapChainExtent.height);
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

    VkFormat swapChainImageFormat{};
    VkExtent2D swapChainExtent{};

    std::vector<VkFramebuffer> swapChainFramebuffers{};
    VkRenderPass renderPass = VK_NULL_HANDLE;

    std::vector<VkImage> depthImages{};
    std::vector<VkDeviceMemory> depthImageMemorys{};
    std::vector<VkImageView> depthImageViews{};
    std::vector<VkImage> swapChainImages{};
    std::vector<VkImageView> swapChainImageViews{};

    VkEngineDevice &device;
    VkExtent2D windowExtent{};

    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::shared_ptr<VkEngineSwapChain> oldSwapChain = nullptr;

    std::vector<VkSemaphore> imageAvailableSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    std::vector<VkFence> inFlightFences{};
    std::vector<VkFence> imagesInFlight{};
    size_t currentFrame = 0;
};

} // namespace vke

#endif // !VKENGINE_SWAPCHAIN_HPP