#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_VULKAN_VERSION 1003000

#include "engine_device.hpp"

#include <vulkan/vulkan_core.h>

#include <set>
#include <sstream>
#include <unordered_set>

#include "utils/logger.hpp"
#include "utils/memory.hpp"


namespace vke {
// local callback functions
namespace {
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             const VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             void* /*pUserData*/) {
	std::ostringstream message;
	message << std::to_string(messageSeverity) << ": " << std::to_string(messageTypes) << ":\n";

	message << "\t"
	        << "messageIDName   = <" << pCallbackData->pMessageIdName << ">\n";
	message << "\t"
	        << "messageIdNumber = " << pCallbackData->messageIdNumber << "\n";
	message << "\t"
	        << "message         = <" << pCallbackData->pMessage << ">\n";
	if (0 < pCallbackData->queueLabelCount) {
		message << std::string("\t") << "Queue Labels:\n";
		for (u32 i = 0; i < pCallbackData->queueLabelCount; i++) {
			message << std::string("\t\t") << "labelName = <" << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
		}
	}
	if (0 < pCallbackData->cmdBufLabelCount) {
		message << std::string("\t") << "CommandBuffer Labels:\n";
		for (u32 i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
			message << std::string("\t\t") << "labelName = <" << pCallbackData->pCmdBufLabels[i].pLabelName << ">\n";
		}
	}
	if (0 < pCallbackData->objectCount) {
		message << std::string("\t") << "Objects:\n";
		for (u32 i = 0; i < pCallbackData->objectCount; i++) {
			message << std::string("\t\t") << "Object " << i << "\n";
			message << std::string("\t\t\t")
			        << "objectType   = " << std::to_string(pCallbackData->pObjects[i].objectType) << "\n";
			message << std::string("\t\t\t") << "objectHandle = " << pCallbackData->pObjects[i].objectHandle << "\n";
			if (pCallbackData->pObjects[i].pObjectName != nullptr) {
				message << std::string("\t\t\t") << "objectName   = <" << pCallbackData->pObjects[i].pObjectName
				        << ">\n";
			}
		}
	}

	fmt::println("{}", message.str());

	return VK_FALSE;
}
}  // namespace

VkResult CreateDebugUtilsMessengerEXT(const VkInstance* const instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger) {
	if (const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
	        vkGetInstanceProcAddr(*instance, "vkCreateDebugUtilsMessengerEXT"));
	    func != nullptr) {
		return func(*instance, pCreateInfo, pAllocator, pDebugMessenger);
	}

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(const VkInstance* const instance,
                                   const VkDebugUtilsMessengerEXT* const debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) {
	if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
	        vkGetInstanceProcAddr(*instance, "vkDestroyDebugUtilsMessengerEXT"));
	    func != nullptr) {
		func(*instance, *debugMessenger, pAllocator);
	}
}

// class member functions
VkEngineDevice::VkEngineDevice(std::shared_ptr<VkEngineWindow> window) : pWindow{std::move(window)} {
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createAllocator();
	createCommandPools();
	createDescriptorPools();
}

VkEngineDevice::~VkEngineDevice() {
	VKINFO("Destroyed device");

	vkDeviceWaitIdle(pDevice);

	pCommandBufferPool.cleanUp();

	mDeletionQueue.flush();

	// Destroy the Vulkan device
	if (pDevice != VK_NULL_HANDLE) {
		vkDestroyDevice(pDevice, nullptr);
		pDevice = VK_NULL_HANDLE;
	}
}

void VkEngineDevice::createCommandPools() {
	const QueueFamilyIndices queueFamilyIndices = findQueueFamilies(&pPhysicalDevice);

	const VkCommandPoolCreateInfo poolInfo{
	    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	    .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	    .queueFamilyIndex = queueFamilyIndices.mGraphicsFamily.value(),
	};

	if (vkCreateCommandPool(pDevice, &poolInfo, nullptr, &pCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}

	pCommandBufferPool = {std::make_shared<VkDevice>(pDevice), std::make_unique<VkCommandPool>(pCommandPool)};
}

void VkEngineDevice::createDescriptorPools() {
	constexpr std::array<VkDescriptorPoolSize, 1> pool_sizes = {
	    {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
	};

	const VkDescriptorPoolCreateInfo pool_info{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	                                           .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
	                                           .maxSets = 1,
	                                           .poolSizeCount = 1,
	                                           .pPoolSizes = pool_sizes.data()};

	VK_CHECK(vkCreateDescriptorPool(pDevice, &pool_info, nullptr, &pDescriptorPool));
	mDeletionQueue.push_function([this]() { vkDestroyDescriptorPool(pDevice, pDescriptorPool, nullptr); });
}

void VkEngineDevice::createInstance() {
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	constexpr VkApplicationInfo appInfo = {
	    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
	    .pApplicationName = "VkEngine",
	    .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
	    .pEngineName = "No Engine",
	    .engineVersion = VK_MAKE_VERSION(1, 0, 0),
	    .apiVersion = VK_API_VERSION_1_3,
	};

	u32 extensionCount = 0;
	auto* const extensions = getRequiredExtensions(&extensionCount);

	VkInstanceCreateInfo createInfo = {
	    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#if __APPLE__
	    .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
#endif
	    .pApplicationInfo = &appInfo,
	    .enabledExtensionCount = extensionCount,
	    .ppEnabledExtensionNames = extensions,
	};

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<u32>(mValidationLayer.size());
		createInfo.ppEnabledLayerNames = mValidationLayer.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	VK_CHECK(vkCreateInstance(&createInfo, nullptr, &pInstance));
	mDeletionQueue.push_function([this]() { vkDestroyInstance(pInstance, nullptr); });

	hasGflwRequiredInstanceExtensions();
	Memory::freeMemory(extensions, extensionCount, MEMORY_TAG_VULKAN);
}

void VkEngineDevice::pickPhysicalDevice() {
	u32 deviceCount = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(pInstance, &deviceCount, nullptr));

	fmt::println("device count: {}", deviceCount);

	auto* devices = Memory::allocMemory<VkPhysicalDevice>(deviceCount, MEMORY_TAG_VULKAN);
	VK_CHECK(vkEnumeratePhysicalDevices(pInstance, &deviceCount, devices));

	for (u32 i = 0; i < deviceCount; ++i) {
		if (isDeviceSuitable(&devices[i])) {
			pPhysicalDevice = devices[i];
			break;
		}
	}

	Memory::freeMemory(devices, deviceCount, MEMORY_TAG_VULKAN);

	if (pPhysicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	vkGetPhysicalDeviceProperties(pPhysicalDevice, &mProperties);
	fmt::println("physical device: {}", mProperties.deviceName);
}

void VkEngineDevice::createLogicalDevice() {
	const std::set uniqueQueueFamilies = {findQueueFamilies(&pPhysicalDevice).mGraphicsFamily.value(),
	                                      findQueueFamilies(&pPhysicalDevice).mPresentFamily.value()};

	auto* queueCreateInfos =
	    Memory::allocMemory<VkDeviceQueueCreateInfo>(uniqueQueueFamilies.size(), MEMORY_TAG_VULKAN);

	constexpr float queuePriority = 1.0f;
	for (const auto queueFamily : uniqueQueueFamilies) {
		const VkDeviceQueueCreateInfo queueCreateInfo{
		    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		    .queueFamilyIndex = queueFamily,
		    .queueCount = 1,
		    .pQueuePriorities = &queuePriority,
		};

		queueCreateInfos[queueFamily] = queueCreateInfo;
	}

	// vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features features12{
	    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
	    .descriptorIndexing = VK_TRUE,
	    .bufferDeviceAddress = VK_TRUE,
	};

	VkPhysicalDeviceVulkan13Features features13{
	    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
	    .pNext = &features12,  // link the 1.2 features to the 1.3 features
	    .synchronization2 = VK_TRUE,
	    .dynamicRendering = VK_TRUE,
	};

	VkPhysicalDeviceFeatures2 deviceFeatures2 = {
	    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
	    .pNext = &features13,  // link the 1.3 features to the 2.0 features
	    .features = {.samplerAnisotropy = VK_TRUE, .pipelineStatisticsQuery = VK_TRUE},
	};

	VkDeviceCreateInfo createInfo = {
	    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	    .pNext = &deviceFeatures2,  // link the 2.0 features to the device create info
	    .queueCreateInfoCount = static_cast<u32>(uniqueQueueFamilies.size()),
	    .pQueueCreateInfos = queueCreateInfos,
	    .enabledExtensionCount = static_cast<u32>(mDeviceExtensions.size()),
	    .ppEnabledExtensionNames = mDeviceExtensions.data(),
	};

	// might not really be necessary anymore because device specific validation
	// layers have been deprecated
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<u32>(mValidationLayer.size()),
		createInfo.ppEnabledLayerNames = mValidationLayer.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	VK_CHECK(vkCreateDevice(pPhysicalDevice, &createInfo, nullptr, &pDevice));
	mDeletionQueue.push_function([this]() { vkDestroyDevice(pDevice, nullptr); });

	vkGetDeviceQueue(pDevice, findQueueFamilies(&pPhysicalDevice).mGraphicsFamily.value(), 0, &pGraphicsQueue);
	vkGetDeviceQueue(pDevice, findQueueFamilies(&pPhysicalDevice).mPresentFamily.value(), 0, &pPresentQueue);

	Memory::freeMemory(queueCreateInfos, uniqueQueueFamilies.size(), MEMORY_TAG_VULKAN);
}

void VkEngineDevice::createAllocator() {
	constexpr VmaVulkanFunctions vulkanFunctions{
	    .vkGetInstanceProcAddr = &vkGetInstanceProcAddr,
	    .vkGetDeviceProcAddr = &vkGetDeviceProcAddr,
	};

	const VmaAllocatorCreateInfo allocatorInfo{
	    .flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
	    .physicalDevice = pPhysicalDevice,
	    .device = pDevice,
	    .pVulkanFunctions = &vulkanFunctions,
	    .instance = pInstance,
	    .vulkanApiVersion = VK_API_VERSION_1_3,
	};

	VK_CHECK(vmaCreateAllocator(&allocatorInfo, &pAllocator));
	mDeletionQueue.push_function([this]() { vmaDestroyAllocator(pAllocator); });
}


void VkEngineDevice::createSurface() {
	pWindow->createWindowSurface(&pInstance, &pSurface);
	mDeletionQueue.push_function([this]() { vkDestroySurfaceKHR(pInstance, pSurface, nullptr); });
}

bool VkEngineDevice::isDeviceSuitable(const VkPhysicalDevice* const device) const {
	const QueueFamilyIndices indices = findQueueFamilies(device);
	const bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		swapChainAdequate =
		    !querySwapChainSupport(device).mFormats.empty() && !querySwapChainSupport(device).mPresentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures{};
	vkGetPhysicalDeviceFeatures(*device, &supportedFeatures);

	VkPhysicalDeviceFeatures2 supportedFeatures2{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
	vkGetPhysicalDeviceFeatures2(*device, &supportedFeatures2);

	return indices.isComplete() && extensionsSupported && swapChainAdequate &&
	       (supportedFeatures.samplerAnisotropy != 0u);
}

void VkEngineDevice::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {
	    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
	    .messageSeverity =
	        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
	    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
	                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
	    .pfnUserCallback = debugCallback,
	    .pUserData = nullptr  // Optional
	};
}

void VkEngineDevice::setupDebugMessenger() {
	if constexpr (!enableValidationLayers) {
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	populateDebugMessengerCreateInfo(createInfo);
	VK_CHECK(CreateDebugUtilsMessengerEXT(&pInstance, &createInfo, nullptr, &pDebugMessenger));
	mDeletionQueue.push_function([this]() { DestroyDebugUtilsMessengerEXT(&pInstance, &pDebugMessenger, nullptr); });
}

bool VkEngineDevice::checkValidationLayerSupport() const {
	u32 layerCount = 0;
	VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

	if (layerCount == 0) {
		throw std::runtime_error("No layers found");
	}

	auto* availableLayers = Memory::allocMemory<VkLayerProperties>(layerCount, MEMORY_TAG_VULKAN);

	VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers));

	for (const auto* const i : mValidationLayer) {
		bool layerFound = false;

		for (size_t j = 0; j < layerCount; ++j) {
			if (strcmp(i, availableLayers[j].layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			delete[] availableLayers;
			return false;
		}
	}

	Memory::freeMemory(availableLayers, layerCount, MEMORY_TAG_VULKAN);

	return true;
}

const char** VkEngineDevice::getRequiredExtensions(u32* extensionCount) {
	u32 glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	u32 additionalExtensionCount = 0;
	if (enableValidationLayers) {
		additionalExtensionCount++;
	}

#if __APPLE__
	additionalExtensionCount += 2;  // For the two additional extensions on Apple platforms.
#endif

	*extensionCount = glfwExtensionCount + additionalExtensionCount;

	auto* const extensions = Memory::allocMemory<const char*>(*extensionCount, MEMORY_TAG_VULKAN);
	memcpy(extensions, glfwExtensions, glfwExtensionCount * sizeof(const char*));

	u32 index = glfwExtensionCount;
	if (enableValidationLayers) {
		extensions[index++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}

#if __APPLE__
	extensions[index++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
	extensions[index++] = VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME;
#endif

	return extensions;
}

void VkEngineDevice::hasGflwRequiredInstanceExtensions() {
	u32 extensionCount = 0;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));

	auto* extensions = Memory::allocMemory<VkExtensionProperties>(extensionCount, MEMORY_TAG_VULKAN);
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions));

	fmt::println("available extensions:");
	std::unordered_set<std::string> available{};
	for (u32 i = 0; i < extensionCount; ++i) {
		fmt::println("\t{}", extensions[i].extensionName);
		available.insert(extensions[i].extensionName);
	}

	fmt::println("required extensions:");

	u32 requiredExtensionCount = 0;
	const char** requiredExtensions = getRequiredExtensions(&requiredExtensionCount);
	for (u32 i = 0; i < requiredExtensionCount; ++i) {
		fmt::println("\t{}", requiredExtensions[i]);
		if (!available.contains(requiredExtensions[i])) {
			throw std::runtime_error("missing required extension");
		}
	}

	Memory::freeMemory(extensions, extensionCount, MEMORY_TAG_VULKAN);
	Memory::freeMemory(requiredExtensions, requiredExtensionCount, MEMORY_TAG_VULKAN);
}

bool VkEngineDevice::checkDeviceExtensionSupport(const VkPhysicalDevice* const device) const {
	u32 extensionCount = 0;
	VK_CHECK(vkEnumerateDeviceExtensionProperties(*device, nullptr, &extensionCount, nullptr));

	auto* availableExtensions = Memory::allocMemory<VkExtensionProperties>(extensionCount, MEMORY_TAG_VULKAN);
	VK_CHECK(vkEnumerateDeviceExtensionProperties(*device, nullptr, &extensionCount, availableExtensions));

	std::set<std::string> requiredExtensions(mDeviceExtensions.data(),
	                                         mDeviceExtensions.data() + mDeviceExtensions.size());

	for (u32 i = 0; i < extensionCount; ++i) {
		requiredExtensions.erase(availableExtensions[i].extensionName);
	}

	Memory::freeMemory(availableExtensions, extensionCount, MEMORY_TAG_VULKAN);
	return requiredExtensions.empty();
}

QueueFamilyIndices VkEngineDevice::findQueueFamilies(const VkPhysicalDevice* const device) const {
	QueueFamilyIndices indices{};

	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, nullptr);

	auto* queueFamilies = Memory::allocMemory<VkQueueFamilyProperties>(queueFamilyCount, MEMORY_TAG_VULKAN);
	vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, queueFamilies);

	for (u32 i = 0; i < queueFamilyCount; ++i) {
		if (queueFamilies[i].queueCount > 0 && (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u) {
			indices.mGraphicsFamily = i;
		}
		VkBool32 presentSupport = 0u;
		VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(*device, i, pSurface, &presentSupport));
		if (queueFamilies[i].queueCount > 0 && presentSupport != 0u) {
			indices.mPresentFamily = i;
		}
		if (indices.isComplete()) {
			break;
		}
	}

	Memory::freeMemory(queueFamilies, queueFamilyCount, MEMORY_TAG_VULKAN);

	return indices;
}

SwapChainSupportDetails VkEngineDevice::querySwapChainSupport(const VkPhysicalDevice* const device) const {
	SwapChainSupportDetails details{};
	VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*device, pSurface, &details.mCapabilities));

	u32 formatCount = 0;
	VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(*device, pSurface, &formatCount, nullptr));

	if (formatCount != 0) {
		details.mFormats.resize(formatCount);
		VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(*device, pSurface, &formatCount, details.mFormats.data()));
	}

	u32 presentModeCount = 0;
	VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(*device, pSurface, &presentModeCount, nullptr));

	if (presentModeCount != 0) {
		details.mPresentModes.resize(presentModeCount);
		VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(*device, pSurface, &presentModeCount,
		                                                   details.mPresentModes.data()));
	}
	return details;
}

VkFormat VkEngineDevice::findSupportedFormat(const std::vector<VkFormat>& candidates, const VkImageTiling tiling,
                                             const VkFormatFeatureFlags features) const {
	for (const auto& format : candidates) {
		VkFormatProperties props{};
		vkGetPhysicalDeviceFormatProperties(pPhysicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}

		if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}
	throw std::runtime_error("failed to find supported format!");
}

u32 VkEngineDevice::findMemoryType(const u32 typeFilter, const VkMemoryPropertyFlags properties) const {
	VkPhysicalDeviceMemoryProperties memProperties{};
	vkGetPhysicalDeviceMemoryProperties(pPhysicalDevice, &memProperties);

	for (u32 i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & 1 << i) != 0u && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}


VkCommandBuffer VkEngineDevice::beginSingleTimeCommands() const {
	auto* const commandBuffer = pCommandBufferPool.getCommandBuffer();

	constexpr VkCommandBufferBeginInfo beginInfo{
	    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

void VkEngineDevice::endSingleTimeCommands(const VkCommandBuffer* const commandBuffer) const {
	vkEndCommandBuffer(*commandBuffer);

	const VkSubmitInfo submitInfo{
	    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = commandBuffer};

	vkQueueSubmit(pGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(pGraphicsQueue);

	pCommandBufferPool.returnCommandBuffer(*commandBuffer);
}

void VkEngineDevice::createBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage,
                                  const VmaMemoryUsage memoryUsage, VkBuffer& buffer,
                                  VmaAllocation& bufferAllocation) const {
	const VkBufferCreateInfo bufferInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	                                    .size = size,
	                                    .usage = usage,
	                                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

	const VmaAllocationCreateInfo allocInfo{.usage = memoryUsage};

	if (vmaCreateBuffer(pAllocator, &bufferInfo, &allocInfo, &buffer, &bufferAllocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}
}

void VkEngineDevice::copyBuffer(const VkBuffer* const srcBuffer, const VkBuffer* const dstBuffer,
                                const VkDeviceSize size) const {
	auto* const commandBuffer = beginSingleTimeCommands();

	const VkBufferCopy copyRegion{
	    .srcOffset = 0,
	    .dstOffset = 0,
	    .size = size,
	};

	vkCmdCopyBuffer(commandBuffer, *srcBuffer, *dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(&commandBuffer);
}


void VkEngineDevice::copyBufferToImage(const VkBuffer* const buffer, const VkImage* const image, const uint32_t width,
                                       const uint32_t height, const uint32_t layerCount) const {
	auto* const commandBuffer = beginSingleTimeCommands();

	const VkBufferImageCopy region{
	    .bufferOffset = 0,
	    .bufferRowLength = 0,
	    .bufferImageHeight = 0,
	    .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	                         .mipLevel = 0,
	                         .baseArrayLayer = 0,
	                         .layerCount = layerCount},
	    .imageOffset = {0, 0, 0},
	    .imageExtent = {width, height, 1},
	};


	vkCmdCopyBufferToImage(commandBuffer, *buffer, *image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	endSingleTimeCommands(&commandBuffer);
}


void VkEngineDevice::createImageWithInfo(const VkImageCreateInfo& imageInfo, VkImage& image,
                                         VmaAllocation& imageMemory) const {
	constexpr VmaAllocationCreateInfo rimg_allocinfo{
	    .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, .usage = VMA_MEMORY_USAGE_AUTO, .priority = 1.0f};

	vmaCreateImage(pAllocator, &imageInfo, &rimg_allocinfo, &image, &imageMemory, nullptr);
}


}  // namespace vke