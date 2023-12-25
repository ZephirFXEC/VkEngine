#define VMA_IMPLEMENTATION
#include "vkEngineDevice.hpp"

// std headers
#include <cstring>
#include <set>
#include <sstream>
#include <unordered_set>

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
		for (uint32_t i = 0; i < pCallbackData->queueLabelCount; i++) {
			message << std::string("\t\t") << "labelName = <" << pCallbackData->pQueueLabels[i].pLabelName << ">\n";
		}
	}
	if (0 < pCallbackData->cmdBufLabelCount) {
		message << std::string("\t") << "CommandBuffer Labels:\n";
		for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; i++) {
			message << std::string("\t\t") << "labelName = <" << pCallbackData->pCmdBufLabels[i].pLabelName << ">\n";
		}
	}
	if (0 < pCallbackData->objectCount) {
		message << std::string("\t") << "Objects:\n";
		for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
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

#ifdef _WIN32
	MessageBox(NULL, message.str().c_str(), "Alert", MB_OK);
#else
	std::cerr << message.str() << '\n';
#endif

	return 0u;
}
}  // namespace

VkResult CreateDebugUtilsMessengerEXT(const VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger) {
	if (const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
	        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
	    func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(const VkInstance instance, const VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) {
	if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
	        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
	    func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

// class member functions
VkEngineDevice::VkEngineDevice(VkEngineWindow& window) : mWindow{window} {
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createCommandPool();
	createAllocator();
}

VkEngineDevice::~VkEngineDevice() {
	// 1. Destroy the command pool
	vkDestroyCommandPool(pDevice, mFrameData.pCommandPool, nullptr);

	// 2. Destroy the allocator
	vmaDestroyAllocator(pAllocator);

	// 3. Destroy the Vulkan device
	vkDestroyDevice(pDevice, nullptr);

	// 4. Optionally destroy the debug messenger if validation layers are enabled
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(pInstance, pDebugMessenger, nullptr);
	}

	// 5. Destroy the surface
	vkDestroySurfaceKHR(pInstance, pSurface, nullptr);

	// 6. Destroy the Vulkan instance
	vkDestroyInstance(pInstance, nullptr);
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

	auto extensions = getRequiredExtensions();

	VkInstanceCreateInfo createInfo = {
	    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	    .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
	    .pApplicationInfo = &appInfo,
	    .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
	    .ppEnabledExtensionNames = extensions.data(),
	};

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
		createInfo.ppEnabledLayerNames = mValidationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
	} else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &pInstance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}

	hasGflwRequiredInstanceExtensions();
}

void VkEngineDevice::pickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(pInstance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}
	std::cout << "Device count: " << deviceCount << '\n';

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(pInstance, &deviceCount, devices.data());

	for (uint32_t i = 0; i < deviceCount; ++i) {
		if (isDeviceSuitable(devices[i])) {
			pPhysicalDevice = devices[i];
			break;
		}
	}

	if (pPhysicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	vkGetPhysicalDeviceProperties(pPhysicalDevice, &mProperties);
	std::cout << "physical device: " << mProperties.deviceName << '\n';
}

void VkEngineDevice::createLogicalDevice() {
	const std::set uniqueQueueFamilies = {findQueueFamilies(pPhysicalDevice).mGraphicsFamily.value(),
	                                      findQueueFamilies(pPhysicalDevice).mPresentFamily.value()};

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(uniqueQueueFamilies.size());

	float queuePriority = 1.0f;
	for (const auto queueFamily : uniqueQueueFamilies) {
		const VkDeviceQueueCreateInfo queueCreateInfo{
		    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		    .queueFamilyIndex = queueFamily,
		    .queueCount = 1,
		    .pQueuePriorities = &queuePriority,
		};

		queueCreateInfos[queueFamily] = queueCreateInfo;
	}

	constexpr VkPhysicalDeviceFeatures deviceFeatures = {
	    .samplerAnisotropy = VK_TRUE,
	};


	VkDeviceCreateInfo createInfo = {
	    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	    .queueCreateInfoCount = static_cast<uint32_t>(uniqueQueueFamilies.size()),
	    .pQueueCreateInfos = queueCreateInfos.data(),
	    .enabledExtensionCount = static_cast<uint32_t>(mDeviceExtensions.size()),
	    .ppEnabledExtensionNames = mDeviceExtensions.data(),
	    .pEnabledFeatures = &deviceFeatures,
	};

	// might not really be necessary anymore because device specific validation
	// layers have been deprecated
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
		createInfo.ppEnabledLayerNames = mValidationLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(pPhysicalDevice, &createInfo, nullptr, &pDevice) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(pDevice, findQueueFamilies(pPhysicalDevice).mGraphicsFamily.value(), 0, &pGraphicsQueue);
	vkGetDeviceQueue(pDevice, findQueueFamilies(pPhysicalDevice).mPresentFamily.value(), 0, &pPresentQueue);
}

void VkEngineDevice::createCommandPool() {
	const VkCommandPoolCreateInfo poolInfo = {
	    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	    .queueFamilyIndex = findPhysicalQueueFamilies().mGraphicsFamily.value(),

	};

	if (vkCreateCommandPool(pDevice, &poolInfo, nullptr, &mFrameData.pCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void VkEngineDevice::createAllocator() {
	VmaVulkanFunctions vulkanFunctions{
	    .vkGetInstanceProcAddr = &vkGetInstanceProcAddr,
	    .vkGetDeviceProcAddr = &vkGetDeviceProcAddr,
	};

	const VmaAllocatorCreateInfo allocatorInfo{
	    .physicalDevice = pPhysicalDevice,
	    .device = pDevice,
	    .pVulkanFunctions = &vulkanFunctions,
	    .instance = pInstance,
	    .vulkanApiVersion = VK_API_VERSION_1_3,
	};

	vmaCreateAllocator(&allocatorInfo, &pAllocator);
}

void VkEngineDevice::createSurface() { mWindow.createWindowSurface(pInstance, &pSurface); }

bool VkEngineDevice::isDeviceSuitable(const VkPhysicalDevice device) const {
	const QueueFamilyIndices indices = findQueueFamilies(device);
	const bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.mFormats.empty() && !swapChainSupport.mPresentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

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

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);
	if (CreateDebugUtilsMessengerEXT(pInstance, &createInfo, nullptr, &pDebugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

bool VkEngineDevice::checkValidationLayerSupport() const {
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	if (layerCount == 0) {
		throw std::runtime_error("No layers found");
	}

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : mValidationLayers) {
		bool layerFound = false;

		for (uint32_t j = 0; j < layerCount; ++j) {
			if (strcmp(layerName, availableLayers[j].layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

// TODO: remove vextor and use pointer arithmetic
std::vector<const char*> VkEngineDevice::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

#if __APPLE__
	extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

	return extensions;
}

void VkEngineDevice::hasGflwRequiredInstanceExtensions() {
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	std::cout << "available extensions:" << '\n';
	std::unordered_set<std::string> available{};
	for (const auto& extension : extensions) {
		std::cout << "\t" << extension.extensionName << '\n';
		available.insert(extension.extensionName);
	}

	std::cout << "required extensions:" << '\n';
	for (const auto& required : getRequiredExtensions()) {
		std::cout << "\t" << required << '\n';
		if (!available.contains(required)) {
			throw std::runtime_error("Missing required glfw extension");
		}
	}
}

bool VkEngineDevice::checkDeviceExtensionSupport(const VkPhysicalDevice device) const {
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(mDeviceExtensions.begin(), mDeviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

QueueFamilyIndices VkEngineDevice::findQueueFamilies(const VkPhysicalDevice device) const {
	QueueFamilyIndices indices{};

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	for (uint32_t i = 0; i < queueFamilyCount; ++i) {
		if (queueFamilies[i].queueCount > 0 && (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u) {
			indices.mGraphicsFamily = i;
		}
		VkBool32 presentSupport = 0u;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, pSurface, &presentSupport);
		if (queueFamilies[i].queueCount > 0 && presentSupport != 0u) {
			indices.mPresentFamily = i;
		}
		if (indices.isComplete()) {
			break;
		}
	}

	return indices;
}

SwapChainSupportDetails VkEngineDevice::querySwapChainSupport(const VkPhysicalDevice device) const {
	SwapChainSupportDetails details{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, pSurface, &details.mCapabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, pSurface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.mFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, pSurface, &formatCount, details.mFormats.data());
	}

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, pSurface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.mPresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, pSurface, &presentModeCount, details.mPresentModes.data());
	}
	return details;
}

VkFormat VkEngineDevice::findSupportedFormat(const std::vector<VkFormat>& candidates, const VkImageTiling tiling,
                                             const VkFormatFeatureFlags features) const {
	for (const VkFormat format : candidates) {
		VkFormatProperties props;
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

uint32_t VkEngineDevice::findMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags properties) const {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(pPhysicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & 1 << i) != 0u && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void VkEngineDevice::createBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage,
                                  const VkMemoryPropertyFlags properties, VkBuffer& buffer,
                                  VkDeviceMemory& bufferMemory) const {
	const VkBufferCreateInfo bufferInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	                                    .size = size,
	                                    .usage = usage,
	                                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

	if (vkCreateBuffer(pDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(pDevice, buffer, &memRequirements);

	const VkMemoryAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	                                     .allocationSize = memRequirements.size,
	                                     .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)};

	if (vkAllocateMemory(pDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(pDevice, buffer, bufferMemory, 0);
}

void VkEngineDevice::beginSingleTimeCommands() {
	const VkCommandBufferAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	                                            .commandPool = mFrameData.pCommandPool,
	                                            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	                                            .commandBufferCount = 1};

	vkAllocateCommandBuffers(pDevice, &allocInfo, &mFrameData.pCommandBuffer);

	constexpr VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	                                             .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

	vkBeginCommandBuffer(mFrameData.pCommandBuffer, &beginInfo);
}

void VkEngineDevice::endSingleTimeCommands() const {
	vkEndCommandBuffer(mFrameData.pCommandBuffer);

	const VkSubmitInfo submitInfo{
	    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &mFrameData.pCommandBuffer};

	vkQueueSubmit(pGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(pGraphicsQueue);

	vkFreeCommandBuffers(pDevice, mFrameData.pCommandPool, 1, &mFrameData.pCommandBuffer);
}

void VkEngineDevice::copyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size) {
	beginSingleTimeCommands();

	const VkBufferCopy copyRegion{.srcOffset = 0,  // Optional
	                              .dstOffset = 0,  // Optional
	                              .size = size};

	vkCmdCopyBuffer(mFrameData.pCommandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands();
}

void VkEngineDevice::copyBufferToImage(const VkBuffer buffer, const VkImage image, const uint32_t width,
                                       const uint32_t height, const uint32_t layerCount) {
	beginSingleTimeCommands();

	const VkBufferImageCopy region{.bufferOffset = 0,
	                               .bufferRowLength = 0,
	                               .bufferImageHeight = 0,

	                               .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	                                                    .mipLevel = 0,
	                                                    .baseArrayLayer = 0,
	                                                    .layerCount = layerCount},

	                               .imageOffset = {0, 0, 0},
	                               .imageExtent = {width, height, 1}};

	vkCmdCopyBufferToImage(mFrameData.pCommandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	endSingleTimeCommands();
}

void VkEngineDevice::createImageWithInfo(const VkImageCreateInfo& imageInfo, const VkMemoryPropertyFlags properties,
                                         VkImage& image, VkDeviceMemory& imageMemory) const {
	if (vkCreateImage(pDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(pDevice, image, &memRequirements);

	const VkMemoryAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	                                     .allocationSize = memRequirements.size,
	                                     .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)};

	if (vkAllocateMemory(pDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	if (vkBindImageMemory(pDevice, image, imageMemory, 0) != VK_SUCCESS) {
		throw std::runtime_error("failed to bind image memory!");
	}
}
}  // namespace vke