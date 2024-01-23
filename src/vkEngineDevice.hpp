#ifndef VKDEVICE_H
#define VKDEVICE_H

#include "vkWindow.hpp"
#include <vulkan/vulkan.hpp>

namespace vke
{

struct SwapChainSupportDetails
{
	vk::SurfaceCapabilitiesKHR mCapabilities{};
	std::vector<vk::SurfaceFormatKHR> mFormats{};
	std::vector<vk::PresentModeKHR> mPresentModes{};
};

struct QueueFamilyIndices
{
	uint32_t mGraphicsFamily{};
	uint32_t mPresentFamily{};
	bool mGraphicsFamilyHasValue = false;
	bool mPresentFamilyHasValue = false;
	[[nodiscard]] bool isComplete() const
	{
		return mGraphicsFamilyHasValue && mPresentFamilyHasValue;
	}
};

class VkEngineDevice
{
public:
#ifdef NDEBUG
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = true;
#endif

	explicit VkEngineDevice(VkWindow& window);
	~VkEngineDevice();

	// Not copyable or movable
	VkEngineDevice(const VkEngineDevice&) = delete;
	void operator=(const VkEngineDevice&) = delete;
	VkEngineDevice(VkEngineDevice&&) = delete;
	VkEngineDevice& operator=(VkEngineDevice&&) = delete;

	[[nodiscard]] const vk::CommandPool& getCommandPool() const
	{
		return pCommandPool;
	}
	[[nodiscard]] const vk::Device& device() const
	{
		return pDevice;
	}
	[[nodiscard]] const vk::SurfaceKHR& surface() const
	{
		return pSurface;
	}
	[[nodiscard]] const vk::Queue& graphicsQueue() const
	{
		return pGraphicsQueue;
	}
	[[nodiscard]] const vk::Queue& presentQueue() const
	{
		return pPresentQueue;
	}

	[[nodiscard]] SwapChainSupportDetails getSwapChainSupport() const
	{
		return querySwapChainSupport(pPhysicalDevice);
	}
	[[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter,
										  vk::MemoryPropertyFlags properties) const;
	[[nodiscard]] QueueFamilyIndices findPhysicalQueueFamilies() const
	{
		return findQueueFamilies(pPhysicalDevice);
	}
	[[nodiscard]] vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates,
												 vk::ImageTiling tiling,
												 vk::FormatFeatureFlags features) const;
	[[nodiscard]] vk::CommandBuffer beginSingleTimeCommands() const;

	// Buffer Helper Functions
	void createBuffer(vk::DeviceSize size,
					  vk::BufferUsageFlags usage,
					  vk::MemoryPropertyFlags properties,
					  vk::Buffer& buffer,
					  vk::DeviceMemory& bufferMemory) const;

	void endSingleTimeCommands(vk::CommandBuffer commandBuffer) const;

	void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) const;

	void copyBufferToImage(vk::Buffer buffer,
						   vk::Image image,
						   uint32_t width,
						   uint32_t height,
						   uint32_t layerCount) const;

	void createImageWithInfo(const vk::ImageCreateInfo& imageInfo,
							 vk::MemoryPropertyFlags properties,
							 vk::Image& image,
							 vk::DeviceMemory& imageMemory) const;

	vk::PhysicalDeviceProperties mProperties{};

private:
	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createCommandPool();

	// helper functions
	[[nodiscard]] bool isDeviceSuitable(vk::PhysicalDevice device) const;
	[[nodiscard]] std::vector<const char*> getRequiredExtensions() const;
	[[nodiscard]] bool checkValidationLayerSupport() const;
	[[nodiscard]] QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device) const;
	[[nodiscard]] bool checkDeviceExtensionSupport(vk::PhysicalDevice device) const;
	[[nodiscard]] SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device) const;
	static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void hasGflwRequiredInstanceExtensions() const;


	vk::Instance pInstance{};
	VkDebugUtilsMessengerEXT pDebugMessenger = VK_NULL_HANDLE;
	vk::PhysicalDevice pPhysicalDevice{};
	VkWindow& mWindow;
	vk::CommandPool pCommandPool{};

	vk::Device pDevice{};
	vk::SurfaceKHR pSurface{};
	vk::Queue pGraphicsQueue{};
	vk::Queue pPresentQueue{};

	const std::vector<const char*> mValidationLayers = {"VK_LAYER_KHRONOS_validation"};
	const std::vector<const char*> mDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
};

} // namespace vke

#endif // VKDEVICE_H