

#define VMA_IMPLEMENTATION
#include <pch.hpp>

#include "vkEngineDevice.hpp"

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

#ifdef _WIN32
	MessageBox(NULL, message.str().c_str(), "Alert", MB_OK);
#else
	fmt::println("{}", message.str());
#endif

	return 0u;
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
VkEngineDevice::VkEngineDevice(VkEngineWindow& window) : mWindow{window} {
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createAllocator();
}

VkEngineDevice::~VkEngineDevice() {
	// 2. Destroy the allocator
	vmaDestroyAllocator(pAllocator);

	// 3. Destroy the Vulkan device
	vkDestroyDevice(pDevice, nullptr);

	// 4. Optionally destroy the debug messenger if validation layers are enabled
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(&pInstance, &pDebugMessenger, nullptr);
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

	u32 extensionCount = 0;
	auto* const extensions = getRequiredExtensions(&extensionCount);

	VkInstanceCreateInfo createInfo = {
	    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	    .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
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

	hasGflwRequiredInstanceExtensions();
	delete[] extensions;
}

void VkEngineDevice::pickPhysicalDevice() {
	u32 deviceCount = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(pInstance, &deviceCount, nullptr));

	fmt::println("device count: {}", deviceCount);

	auto* devices = new VkPhysicalDevice[deviceCount]{};
	VK_CHECK(vkEnumeratePhysicalDevices(pInstance, &deviceCount, devices));

	for (u32 i = 0; i < deviceCount; ++i) {
		if (isDeviceSuitable(&devices[i])) {
			pPhysicalDevice = devices[i];
			break;
		}
	}

	delete[] devices;

	if (pPhysicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	vkGetPhysicalDeviceProperties(pPhysicalDevice, &mProperties);
	fmt::println("physical device: {}", mProperties.deviceName);
}

void VkEngineDevice::createLogicalDevice() {
	const std::set uniqueQueueFamilies = {findQueueFamilies(&pPhysicalDevice).mGraphicsFamily.value(),
	                                      findQueueFamilies(&pPhysicalDevice).mPresentFamily.value()};

	auto* queueCreateInfos = new VkDeviceQueueCreateInfo[uniqueQueueFamilies.size()]{};

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

	// vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features features12{
	    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
	    .bufferDeviceAddress = VK_TRUE,
	    .descriptorIndexing = VK_TRUE,
	};

	VkPhysicalDeviceVulkan13Features features13{
	    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
	    .dynamicRendering = VK_TRUE,
	    .synchronization2 = VK_TRUE,
	    .pNext = &features12,  // link the 1.2 features to the 1.3 features
	};

	VkPhysicalDeviceFeatures2 deviceFeatures2 = {
	    .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
	    .features =
	        {
	            .samplerAnisotropy = VK_TRUE,
	        },
	    .pNext = &features13,  // link the 1.3 features to the 2.0 features

	};

	VkDeviceCreateInfo createInfo = {
	    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	    .queueCreateInfoCount = static_cast<u32>(uniqueQueueFamilies.size()),
	    .pQueueCreateInfos = queueCreateInfos,
	    .enabledExtensionCount = static_cast<u32>(mDeviceExtensions.size()),
	    .ppEnabledExtensionNames = mDeviceExtensions.data(),
	    .pNext = &deviceFeatures2,  // link the 2.0 features to the device create info
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

	vkGetDeviceQueue(pDevice, findQueueFamilies(&pPhysicalDevice).mGraphicsFamily.value(), 0, &pGraphicsQueue);
	vkGetDeviceQueue(pDevice, findQueueFamilies(&pPhysicalDevice).mPresentFamily.value(), 0, &pPresentQueue);

	delete[] queueCreateInfos;
}

void VkEngineDevice::createAllocator() {
	VmaVulkanFunctions vulkanFunctions{
	    .vkGetInstanceProcAddr = &vkGetInstanceProcAddr,
	    .vkGetDeviceProcAddr = &vkGetDeviceProcAddr,
	};

	const VmaAllocatorCreateInfo allocatorInfo{
	    .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
	    .physicalDevice = pPhysicalDevice,
	    .device = pDevice,
	    .pVulkanFunctions = &vulkanFunctions,
	    .instance = pInstance,
	    .vulkanApiVersion = VK_API_VERSION_1_3,
	};

	VK_CHECK(vmaCreateAllocator(&allocatorInfo, &pAllocator));
}

void VkEngineDevice::createSurface() { mWindow.createWindowSurface(&pInstance, &pSurface); }

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
}

bool VkEngineDevice::checkValidationLayerSupport() const {
	u32 layerCount = 0;
	VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

	if (layerCount == 0) {
		throw std::runtime_error("No layers found");
	}

	auto* availableLayers = new VkLayerProperties[layerCount]{};
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

	delete[] availableLayers;
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

	auto* const extensions = new const char*[*extensionCount];
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

	auto* extensions = new VkExtensionProperties[extensionCount]{};
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

	delete[] extensions;
	delete[] requiredExtensions;
}

bool VkEngineDevice::checkDeviceExtensionSupport(const VkPhysicalDevice* const device) const {
	u32 extensionCount = 0;
	VK_CHECK(vkEnumerateDeviceExtensionProperties(*device, nullptr, &extensionCount, nullptr));

	auto* availableExtensions = new VkExtensionProperties[extensionCount]{};
	VK_CHECK(vkEnumerateDeviceExtensionProperties(*device, nullptr, &extensionCount, availableExtensions));

	std::set<std::string> requiredExtensions(mDeviceExtensions.data(),
	                                         mDeviceExtensions.data() + mDeviceExtensions.size());

	for (u32 i = 0; i < extensionCount; ++i) {
		requiredExtensions.erase(availableExtensions[i].extensionName);
	}

	delete[] availableExtensions;
	return requiredExtensions.empty();
}

QueueFamilyIndices VkEngineDevice::findQueueFamilies(const VkPhysicalDevice* const device) const {
	QueueFamilyIndices indices{};

	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(*device, &queueFamilyCount, nullptr);

	auto* queueFamilies = new VkQueueFamilyProperties[queueFamilyCount]{};
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

	delete[] queueFamilies;

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
}  // namespace vke