#pragma once

#include <array>

#include "engine_window.hpp"

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
	[[nodiscard]] const VkPhysicalDeviceProperties& getPhysicalDeviceProperties() const { return mProperties; }
	[[nodiscard]] const VkInstance& getInstance() const { return pInstance; }
	[[nodiscard]] const VkSurfaceKHR& getSurface() const { return pSurface; }
	[[nodiscard]] const VkQueue& getGraphicsQueue() const { return pGraphicsQueue; }
	[[nodiscard]] const VkQueue& getPresentQueue() const { return pPresentQueue; }
	[[nodiscard]] const VkDevice& getDevice() const { return pDevice; }
	[[nodiscard]] const VmaAllocator& getAllocator() const { return pAllocator; }
	[[nodiscard]] const VkPhysicalDevice& getPhysicalDevice() const { return pPhysicalDevice; }
	[[nodiscard]] const VkDescriptorPool& getDescriptorPool() const { return pDescriptorPool; }

	[[nodiscard]] SwapChainSupportDetails getSwapChainSupport() const {
		return querySwapChainSupport(&pPhysicalDevice);
	}

	[[nodiscard]] u32 findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties) const;

	[[nodiscard]] QueueFamilyIndices findPhysicalQueueFamilies() const { return findQueueFamilies(&pPhysicalDevice); }

	[[nodiscard]] VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
	                                           VkFormatFeatureFlags features) const;

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
	                                  VkBuffer& buffer, VmaAllocation& bufferAllocation) const;

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

	[[nodiscard]] static const char** getRequiredExtensions(u32* extensionCount);

	[[nodiscard]] bool checkValidationLayerSupport() const;

	QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice* device) const;

	static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	static void hasGflwRequiredInstanceExtensions();

	bool checkDeviceExtensionSupport(const VkPhysicalDevice* device) const;

	SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice* device) const;

	const VkEngineWindow& mWindow;
	VkDevice pDevice = VK_NULL_HANDLE;

	VmaAllocator pAllocator = VK_NULL_HANDLE;

	VkDescriptorPool pDescriptorPool = VK_NULL_HANDLE;
	VkInstance pInstance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT pDebugMessenger = VK_NULL_HANDLE;
	VkPhysicalDevice pPhysicalDevice = VK_NULL_HANDLE;

	VkSurfaceKHR pSurface = VK_NULL_HANDLE;
	VkQueue pGraphicsQueue = VK_NULL_HANDLE;
	VkQueue pPresentQueue = VK_NULL_HANDLE;

	const std::array<const char*, 1> mValidationLayer{"VK_LAYER_KHRONOS_validation"};

	// TODO: make it more bullet proof for cross platform
	const std::array<const char*, 1> mDeviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME
#ifdef __APPLE__
	                                                   ,
	                                                   "VK_KHR_portability_subset", "VK_KHR_buffer_device_address"
#endif
	};
};
}  // namespace vke