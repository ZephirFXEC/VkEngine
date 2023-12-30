#pragma once

#include "utils/utility.hpp"
#include "vkEngineWindow.hpp"

#ifdef NDEBUG
static constexpr bool enableValidationLayers = false;
#else
static constexpr bool enableValidationLayers = true;
#endif

namespace vke {
class VkEngineDevice {
   public:
	explicit VkEngineDevice() = delete;

	explicit VkEngineDevice(VkEngineWindow& window);

	~VkEngineDevice();

	// Not copyable or movable
	VkEngineDevice(const VkEngineDevice&) = delete;

	VkEngineDevice& operator=(const VkEngineDevice&) = delete;

	VkEngineDevice(VkEngineDevice&&) = delete;

	VkEngineDevice& operator=(VkEngineDevice&&) = delete;

	// TYPE				NAME			VARIABLE //
	GETTERS(VkPhysicalDevice, PhysicalDevice, pPhysicalDevice)
	GETTERS(VkInstance, Instance, pInstance)
	GETTERS(VkSurfaceKHR, Surface, pSurface)
	GETTERS(VkQueue, GraphicsQueue, pGraphicsQueue)
	GETTERS(VkQueue, PresentQueue, pPresentQueue)
	GETTERS(VkDevice, Device, pDevice)
	GETTERS(VmaAllocator, Allocator, pAllocator)

	[[nodiscard]] SwapChainSupportDetails getSwapChainSupport() const {
		return querySwapChainSupport(&pPhysicalDevice);
	}

	[[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

	[[nodiscard]] QueueFamilyIndices findPhysicalQueueFamilies() const { return findQueueFamilies(&pPhysicalDevice); }

	[[nodiscard]] VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
	                                           VkFormatFeatureFlags features) const;

	VkPhysicalDeviceProperties mProperties{};

   private:
	void createInstance();

	void setupDebugMessenger();

	void createSurface();

	void pickPhysicalDevice();

	void createLogicalDevice();

	void createAllocator();

	// helper functions
	[[nodiscard]] bool isDeviceSuitable(const VkPhysicalDevice* device) const;

	[[nodiscard]] static const char** getRequiredExtensions(uint32_t* extensionCount);

	[[nodiscard]] bool checkValidationLayerSupport() const;

	QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice* device) const;

	static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	static void hasGflwRequiredInstanceExtensions();

	bool checkDeviceExtensionSupport(const VkPhysicalDevice* device) const;

	SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice* device) const;

	const VkEngineWindow& mWindow;
	VkDevice pDevice = VK_NULL_HANDLE;

	VmaAllocator pAllocator = VK_NULL_HANDLE;

	VkInstance pInstance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT pDebugMessenger = VK_NULL_HANDLE;
	VkPhysicalDevice pPhysicalDevice = VK_NULL_HANDLE;

	VkSurfaceKHR pSurface = VK_NULL_HANDLE;
	VkQueue pGraphicsQueue = VK_NULL_HANDLE;
	VkQueue pPresentQueue = VK_NULL_HANDLE;

	const std::array<const char*, 1> mValidationLayer{"VK_LAYER_KHRONOS_validation"};
	const std::array<const char*, 3> mDeviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME
#ifdef __APPLE__
	                                                   ,
	                                                   "VK_KHR_portability_subset", "VK_KHR_buffer_device_address"
#endif
	};
};
}  // namespace vke