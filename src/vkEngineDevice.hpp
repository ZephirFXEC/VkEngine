#ifndef VKDEVICE_H
#define VKDEVICE_H

#include "vkWindow.hpp"
#include <vector>

namespace vke {

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR mCapabilities{};
    std::vector<VkSurfaceFormatKHR> mFormats{};
    std::vector<VkPresentModeKHR> mPresentModes{};
};

struct QueueFamilyIndices {
    uint32_t mGraphicsFamily{};
    uint32_t mPresentFamily{};
    bool mGraphicsFamilyHasValue = false;
    bool mPresentFamilyHasValue = false;
    [[nodiscard]] bool isComplete() const {
        return mGraphicsFamilyHasValue && mPresentFamilyHasValue;
    }
};

class VkEngineDevice {
  public:
#ifdef NDEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = true;
#endif

    explicit VkEngineDevice(VkWindow &window);
    ~VkEngineDevice();

    // Not copyable or movable
    VkEngineDevice(const VkEngineDevice &) = delete;
    void operator=(const VkEngineDevice &) = delete;
    VkEngineDevice(VkEngineDevice &&) = delete;
    VkEngineDevice &operator=(VkEngineDevice &&) = delete;

    const VkCommandPool &getCommandPool() const { return pCommandPool; }
    const VkDevice &device() const { return pDevice; }
    const VkSurfaceKHR &surface() const { return pSurface; }
    const VkQueue &graphicsQueue() const { return pGraphicsQueue; }
    const VkQueue &presentQueue() const { return pPresentQueue; }

    SwapChainSupportDetails getSwapChainSupport() const { return querySwapChainSupport(pPhysicalDevice); }
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const ;
    QueueFamilyIndices findPhysicalQueueFamilies() const { return findQueueFamilies(pPhysicalDevice); }
    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features) const ;

    // Buffer Helper Functions
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      VkBuffer &buffer, VkDeviceMemory &bufferMemory) const;

    VkCommandBuffer beginSingleTimeCommands() const;

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,
                           uint32_t layerCount) const;

    void createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties,
                             VkImage &image, VkDeviceMemory &imageMemory) const;

    VkPhysicalDeviceProperties mProperties{};

  private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    // helper functions
    bool isDeviceSuitable(VkPhysicalDevice device) const;
    std::vector<const char *> getRequiredExtensions() const;
    bool checkValidationLayerSupport() const;
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void hasGflwRequiredInstanceExtensions() const;
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

    VkInstance pInstance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT pDebugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice pPhysicalDevice = VK_NULL_HANDLE;
    VkWindow &mWindow;
    VkCommandPool pCommandPool = VK_NULL_HANDLE;

    VkDevice pDevice = VK_NULL_HANDLE;
    VkSurfaceKHR pSurface = VK_NULL_HANDLE;
    VkQueue pGraphicsQueue = VK_NULL_HANDLE;
    VkQueue pPresentQueue = VK_NULL_HANDLE;

    const std::vector<const char *> mValidationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> mDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                                        "VK_KHR_portability_subset"};
};

} // namespace vke

#endif // VKDEVICE_H