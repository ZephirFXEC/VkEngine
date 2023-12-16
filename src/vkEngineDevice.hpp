#ifndef VKDEVICE_H
#define VKDEVICE_H

#include "vkWindow.hpp"
#include <vector>

namespace vke {

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
    uint32_t graphicsFamily{};
    uint32_t presentFamily{};
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    [[nodiscard]] bool isComplete() const {
        return graphicsFamilyHasValue && presentFamilyHasValue;
    }
};

class VkEngineDevice {
  public:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
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

    const VkCommandPool& getCommandPool() const { return commandPool; }
    const VkDevice& device() const { return device_; }
    const VkSurfaceKHR& surface() const { return surface_; }
    const VkQueue& graphicsQueue() const { return graphicsQueue_; }
    const VkQueue& presentQueue() const { return presentQueue_; }

    SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features);

    // Buffer Helper Functions
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                      VkBuffer &buffer, VkDeviceMemory &bufferMemory);

    VkCommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,
                           uint32_t layerCount);

    void createImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties,
                             VkImage &image, VkDeviceMemory &imageMemory);

    VkPhysicalDeviceProperties properties{};

  private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    // helper functions
    bool isDeviceSuitable(VkPhysicalDevice device);
    std::vector<const char *> getRequiredExtensions();
    bool checkValidationLayerSupport();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void hasGflwRequiredInstanceExtensions();
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VkInstance instance{};
    VkDebugUtilsMessengerEXT debugMessenger{};
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkWindow &window;
    VkCommandPool commandPool{};

    VkDevice device_{};
    VkSurfaceKHR surface_{};
    VkQueue graphicsQueue_{};
    VkQueue presentQueue_{};

    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

} // namespace vke

#endif // VKDEVICE_H