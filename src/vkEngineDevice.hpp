#ifndef VKDEVICE_HPP
#define VKDEVICE_HPP

#include <functional>
#include <deque>
#include <ranges>

#include "vkEngineWindow.hpp"

#include <vk_mem_alloc.h>


namespace vke
{

struct DeletionQueue
{
	std::deque<std::function<void()>> mDeletionQueue{};

	void push_function(std::function<void()>&& function)
	{
		mDeletionQueue.push_back(std::move(function));
	}

	void flush()
	{
		// reverse iterate the deletion queue to execute all the functions
		for(auto & it : std::ranges::reverse_view(mDeletionQueue))
		{
			it(); // call functors
		}

		mDeletionQueue.clear();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR mCapabilities{};
	std::vector<VkSurfaceFormatKHR> mFormats{};
	std::vector<VkPresentModeKHR> mPresentModes{};
};

struct QueueFamilyIndices
{
	std::optional<uint32_t> mGraphicsFamily;
	std::optional<uint32_t> mPresentFamily;

	[[nodiscard]] bool isComplete() const
	{
		return mGraphicsFamily.has_value() && mPresentFamily.has_value();
	}
};

class VkEngineDevice
{
public:
#ifdef NDEBUG
	static constexpr bool enableValidationLayers = false;
#else
	static constexpr bool enableValidationLayers = true;
#endif

	explicit VkEngineDevice() = delete;
	explicit VkEngineDevice(VkEngineWindow& window);

	~VkEngineDevice();

	// Not copyable or movable
	VkEngineDevice(const VkEngineDevice&) = delete;
	VkEngineDevice& operator=(const VkEngineDevice&) = delete;
	VkEngineDevice(VkEngineDevice&&) = delete;
	VkEngineDevice& operator=(VkEngineDevice&&) = delete;

	[[nodiscard]] const VkCommandPool& getCommandPool() const
	{
		return pCommandPool;
	}

	[[nodiscard]] const VkDevice& device() const
	{
		return pDevice;
	}

	[[nodiscard]] const VkSurfaceKHR& surface() const
	{
		return pSurface;
	}

	[[nodiscard]] const VkQueue& graphicsQueue() const
	{
		return pGraphicsQueue;
	}

	[[nodiscard]] const VkQueue& presentQueue() const
	{
		return pPresentQueue;
	}

	[[nodiscard]] SwapChainSupportDetails getSwapChainSupport() const
	{
		return querySwapChainSupport(pPhysicalDevice);
	}

	[[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter,
										  VkMemoryPropertyFlags properties) const;

	[[nodiscard]] QueueFamilyIndices findPhysicalQueueFamilies() const
	{
		return findQueueFamilies(pPhysicalDevice);
	}

	[[nodiscard]] DeletionQueue& getDeletionQueue()
	{
		return mDeletionQueue;
	}

	[[nodiscard]] VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
											   VkImageTiling tiling,
											   VkFormatFeatureFlags features) const;

	[[nodiscard]] VkCommandBuffer beginSingleTimeCommands() const;

	// Buffer Helper Functions
	void createBuffer(VkDeviceSize size,
					  VkBufferUsageFlags usage,
					  VkMemoryPropertyFlags properties,
					  VkBuffer& buffer,
					  VkDeviceMemory& bufferMemory) const;

	void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;

	void copyBufferToImage(
		VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) const;

	void createImageWithInfo(const VkImageCreateInfo& imageInfo,
							 VkMemoryPropertyFlags properties,
							 VkImage& image,
							 VkDeviceMemory& imageMemory) const;

	VkPhysicalDeviceProperties mProperties{};

private:
	void createInstance();

	void setupDebugMessenger();

	void createSurface();

	void pickPhysicalDevice();

	void createLogicalDevice();

	void createCommandPool();



	// helper functions
	[[nodiscard]] bool isDeviceSuitable(VkPhysicalDevice device) const;

	[[nodiscard]] static std::vector<const char*> getRequiredExtensions();

	[[nodiscard]] bool checkValidationLayerSupport() const;

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;

	static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	static void hasGflwRequiredInstanceExtensions();

	bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

	DeletionQueue mDeletionQueue{};
	VkDevice pDevice = VK_NULL_HANDLE;

	VkInstance pInstance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT pDebugMessenger = VK_NULL_HANDLE;
	VkPhysicalDevice pPhysicalDevice = VK_NULL_HANDLE;
	VkEngineWindow& mWindow;
	VkCommandPool pCommandPool = VK_NULL_HANDLE;

	VkSurfaceKHR pSurface = VK_NULL_HANDLE;
	VkQueue pGraphicsQueue = VK_NULL_HANDLE;
	VkQueue pPresentQueue = VK_NULL_HANDLE;

	const std::vector<const char*> mValidationLayers = {"VK_LAYER_KHRONOS_validation"};
	const std::vector<const char*> mDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
														"VK_KHR_portability_subset"};
};

} // namespace vke

#endif // VKDEVICE_HPP