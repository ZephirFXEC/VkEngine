#pragma once

#include <array>
#include <memory>
#include <mutex>

#include "engine_window.hpp"

#ifdef NDEBUG
static constexpr bool enableValidationLayers = false;
#else
static constexpr bool enableValidationLayers = true;
#endif

namespace vke {


class VkCommandBufferPool {
   public:
	VkCommandBufferPool() = default;
	VkCommandBufferPool(std::shared_ptr<VkDevice> device, std::unique_ptr<VkCommandPool> commandPool)
	    : pDevice(std::move(device)), pCommandPool(std::move(commandPool)) {}

	// Move constructor
	VkCommandBufferPool(VkCommandBufferPool&& other) noexcept
	    : pDevice(std::move(other.pDevice)),
	      pCommandPool(std::move(other.pCommandPool)),
	      pCommandBuffers(std::move(other.pCommandBuffers)) {}

	// Move assignment operator
	VkCommandBufferPool& operator=(VkCommandBufferPool&& other) noexcept {
		if (this != &other) {
			cleanUp();
			pDevice = std::move(other.pDevice);
			pCommandPool = std::move(other.pCommandPool);
			pCommandBuffers = std::move(other.pCommandBuffers);
		}
		return *this;
	}

	// Delete copy constructor and copy assignment operator
	VkCommandBufferPool(const VkCommandBufferPool&) = delete;
	VkCommandBufferPool& operator=(const VkCommandBufferPool&) = delete;


	VkCommandBuffer getCommandBuffer() const {
		std::lock_guard<std::mutex> lock(bufferMutex);
		if (!pCommandBuffers.empty()) {
			const VkCommandBuffer cmd = pCommandBuffers.back();
			pCommandBuffers.pop_back();
			return cmd;
		}

		const VkCommandBufferAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		                                               .commandPool = *pCommandPool,
		                                               .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		                                               .commandBufferCount = 1};

		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
		if (vkAllocateCommandBuffers(*pDevice, &allocInfo, &commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffer");
		}
		return commandBuffer;
	}

	void returnCommandBuffer(const VkCommandBuffer commandBuffer) const {
		std::lock_guard<std::mutex> lock(bufferMutex);
		pCommandBuffers.push_back(commandBuffer);
	}

	void cleanUp() const {
		if (pDevice && pCommandPool) {
			std::lock_guard<std::mutex> lock(bufferMutex);
			for (VkCommandBuffer cmd : pCommandBuffers) {
				vkFreeCommandBuffers(*pDevice, *pCommandPool, 1, &cmd);
			}
			pCommandBuffers.clear();
		}
	}

   private:
	std::shared_ptr<VkDevice> pDevice{};
	std::unique_ptr<VkCommandPool> pCommandPool{};
	mutable std::vector<VkCommandBuffer> pCommandBuffers{};
	mutable std::mutex bufferMutex{};
};


class VkEngineDevice {
   public:
	explicit VkEngineDevice() = delete;
	explicit VkEngineDevice(std::shared_ptr<VkEngineWindow> window);

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
	[[nodiscard]] const VkCommandPool& getCommandPool() const { return pCommandPool; }


	[[nodiscard]] SwapChainSupportDetails getSwapChainSupport() const {
		return querySwapChainSupport(&pPhysicalDevice);
	}

	[[nodiscard]] u32 findMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties) const;

	[[nodiscard]] QueueFamilyIndices findPhysicalQueueFamilies() const { return findQueueFamilies(&pPhysicalDevice); }

	[[nodiscard]] VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
	                                           VkFormatFeatureFlags features) const;

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& buffer,
	                  VmaAllocation& bufferAllocation) const;

	// Buffer Helper Functions
	VkCommandBuffer beginSingleTimeCommands() const;
	void endSingleTimeCommands(const VkCommandBuffer* commandBuffer) const;
	void copyBuffer(const VkBuffer* srcBuffer, const VkBuffer* dstBuffer, VkDeviceSize size) const;
	void copyBufferToImage(const VkBuffer* buffer, const VkImage* image, uint32_t width, uint32_t height,
	                       uint32_t layerCount) const;
	void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkImage& image, VmaAllocation& imageMemory) const;

	VkPhysicalDeviceProperties mProperties{};

   private:
	void createInstance();

	void setupDebugMessenger();

	void createSurface();

	void pickPhysicalDevice();

	void createLogicalDevice();

	void createCommandPools();

	void createDescriptorPools();

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

	const std::shared_ptr<VkEngineWindow> pWindow{};
	VkDevice pDevice = VK_NULL_HANDLE;

	VmaAllocator pAllocator = VK_NULL_HANDLE;
	VkCommandBufferPool pCommandBufferPool{};
	VkCommandPool pCommandPool = VK_NULL_HANDLE;
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