#pragma once

#include "utils/utility.hpp"
#include "vkEngineWindow.hpp"

#ifdef NDEBUG
static constexpr bool enableValidationLayers = false;
#else
static constexpr bool enableValidationLayers = true;
#endif

namespace vke {
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR mCapabilities{};
	std::vector<VkSurfaceFormatKHR> mFormats{};
	std::vector<VkPresentModeKHR> mPresentModes{};
};

struct QueueFamilyIndices {
	std::optional<uint32_t> mGraphicsFamily;
	std::optional<uint32_t> mPresentFamily;

	[[nodiscard]] bool isComplete() const { return mGraphicsFamily.has_value() && mPresentFamily.has_value(); }
};

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

	NDC_INLINE const VkCommandPool& getCommandPool() const { return mFrameData.pCommandPool; }

	NDC_INLINE const VkDevice& getDevice() const { return pDevice; }

	NDC_INLINE const VmaAllocator& getAllocator() const { return pAllocator; }

	NDC_INLINE const VkSurfaceKHR& getSurface() const { return pSurface; }

	NDC_INLINE const VkQueue& getGraphicsQueue() const { return pGraphicsQueue; }

	NDC_INLINE const VkQueue& getPresentQueue() const { return pPresentQueue; }

	[[nodiscard]] SwapChainSupportDetails getSwapChainSupport() const {
		return querySwapChainSupport(&pPhysicalDevice);
	}

	[[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

	[[nodiscard]] QueueFamilyIndices findPhysicalQueueFamilies() const { return findQueueFamilies(&pPhysicalDevice); }

	[[nodiscard]] VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
	                                           VkFormatFeatureFlags features) const;

	// Buffer Helper Functions
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
	                  Alloc& bufferMemory) const;

	void copyBufferToImage(const VkBuffer* buffer, const VkImage* image, uint32_t width, uint32_t height,
	                       uint32_t layerCount);

	void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image,
	                         VkDeviceMemory& imageMemory) const;

	VkPhysicalDeviceProperties mProperties{};

   private:
	void createInstance();

	void setupDebugMessenger();

	void createSurface();

	void pickPhysicalDevice();

	void createLogicalDevice();

	void createCommandPool();

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

	FrameData mFrameData{};

	const std::array<const char*, 1> mValidationLayer{"VK_LAYER_KHRONOS_validation"};
	const std::array<const char*, 2> mDeviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME
#ifdef __APPLE__
	                                                   ,
	                                                   "VK_KHR_portability_subset"
#endif
	};
};
}  // namespace vke