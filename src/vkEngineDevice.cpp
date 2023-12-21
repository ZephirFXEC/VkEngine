#include "vkEngineDevice.hpp"

// std headers
#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

namespace vke
{

// local callback functions
namespace
{
VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
			  VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
			  const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
			  void* /*pUserData*/)
{

	// Log debug message
	if((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0)
	{
		std::cerr << "WARNING: " << callback_data->pMessage << "\n";
	}
	else if((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0)
	{
		std::cerr << "ERROR: " << callback_data->pMessage << "\n";
	}
	return VK_FALSE;
}
} // namespace

VkResult CreateDebugUtilsMessengerEXT(const vk::Instance instance,
									  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
									  const VkAllocationCallbacks* pAllocator,
									  VkDebugUtilsMessengerEXT* pDebugMessenger)
{

	if(const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
		   vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
	   func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(const vk::Instance instance,
								   const VkDebugUtilsMessengerEXT debugMessenger,
								   const VkAllocationCallbacks* pAllocator)
{

	if(const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
		   vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
	   func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

// class member functions
VkEngineDevice::VkEngineDevice(VkWindow& window)
	: mWindow{window}
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createCommandPool();
}

VkEngineDevice::~VkEngineDevice()
{
	pDevice.destroyCommandPool(pCommandPool, nullptr);
	pDevice.destroy(nullptr);

	if(enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(pInstance, pDebugMessenger, nullptr);
	}

	pInstance.destroySurfaceKHR(pSurface, nullptr);
	pInstance.destroy(nullptr);
}

void VkEngineDevice::createInstance()
{
	if(enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	constexpr vk::ApplicationInfo appInfo("VkEngine",
										  VK_MAKE_VERSION(1, 0, 0),
										  "No Engine",
										  VK_MAKE_VERSION(1, 0, 0),
										  VK_API_VERSION_1_2);

	const auto extensions = getRequiredExtensions();

	vk::InstanceCreateInfo createInfo(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
									  &appInfo,
									  0,
									  nullptr,
									  static_cast<uint32_t>(extensions.size()),
									  extensions.data());

	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if(enableValidationLayers)
	{
		createInfo.setEnabledLayerCount(static_cast<uint32_t>(mValidationLayers.size()));
		createInfo.setPpEnabledLayerNames(mValidationLayers.data());

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.setPNext(&debugCreateInfo);
	}
	else
	{
		createInfo.setEnabledLayerCount(0);
		createInfo.setPNext(nullptr);
	}

	if(pInstance = vk::createInstance(createInfo); pInstance == nullptr)
	{
		throw std::runtime_error("failed to create instance!");
	}

	hasGflwRequiredInstanceExtensions();
}

void VkEngineDevice::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	if(const vk::Result r = pInstance.enumeratePhysicalDevices(&deviceCount, nullptr);
	   deviceCount == 0 || r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to enumerate physical devices!");
	}

	std::cout << "Device count: " << deviceCount << '\n';
	auto* devices = new vk::PhysicalDevice[deviceCount];

	if(const vk::Result r = pInstance.enumeratePhysicalDevices(&deviceCount, devices);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to enumerate physical devices!");
	}

	for(uint32_t i = 0; i < deviceCount; ++i)
	{
		if(isDeviceSuitable(devices[i]))
		{
			pPhysicalDevice = devices[i];
			break;
		}
	}

	if(pPhysicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	pPhysicalDevice.getProperties(&mProperties);

	std::cout << "physical device: " << mProperties.deviceName << '\n';
	delete[] devices;
}

void VkEngineDevice::createLogicalDevice()
{

	const std::set uniqueQueueFamilies = {findQueueFamilies(pPhysicalDevice).mGraphicsFamily,
										  findQueueFamilies(pPhysicalDevice).mPresentFamily};

	auto* queueCreateInfos = new vk::DeviceQueueCreateInfo[uniqueQueueFamilies.size()];

	constexpr float queuePriority = 1.0f;
	for(const uint32_t queueFamily : uniqueQueueFamilies)
	{

		const vk::DeviceQueueCreateInfo queueCreateInfo(vk::DeviceQueueCreateFlags(),
												  queueFamily,
												  1,
												  &queuePriority);

		queueCreateInfos[queueFamily] = queueCreateInfo;
	}

	vk::PhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.setSamplerAnisotropy(VK_TRUE);

	vk::DeviceCreateInfo createInfo({},
		static_cast<uint32_t>(uniqueQueueFamilies.size()),
		queueCreateInfos,
		0,
		nullptr,
		static_cast<uint32_t>(mDeviceExtensions.size()),
		mDeviceExtensions.data(),
		&deviceFeatures);

	// might not really be necessary anymore because device specific validation
	// layers have been deprecated
	if(enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(mValidationLayers.size());
		createInfo.ppEnabledLayerNames = mValidationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if(const vk::Result r = pPhysicalDevice.createDevice(&createInfo, nullptr, &pDevice);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	pDevice.getQueue(findQueueFamilies(pPhysicalDevice).mGraphicsFamily, 0, &pGraphicsQueue);
	pDevice.getQueue(findQueueFamilies(pPhysicalDevice).mPresentFamily, 0, &pPresentQueue);

	delete[] queueCreateInfos;
}

void VkEngineDevice::createCommandPool()
{

	const vk::CommandPoolCreateInfo poolInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer
		| vk::CommandPoolCreateFlagBits::eTransient,
		findQueueFamilies(pPhysicalDevice).mGraphicsFamily);

	if(const vk::Result r = pDevice.createCommandPool(&poolInfo, nullptr, &pCommandPool);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

void VkEngineDevice::createSurface()
{
	mWindow.createWindowSurface(pInstance, &pSurface);
}

bool VkEngineDevice::isDeviceSuitable(const vk::PhysicalDevice device) const
{

	const QueueFamilyIndices indices = findQueueFamilies(device);
	const bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if(extensionsSupported)
	{
		const SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate =
			!swapChainSupport.mFormats.empty() && !swapChainSupport.mPresentModes.empty();
	}

	vk::PhysicalDeviceFeatures supportedFeatures;
	device.getFeatures(&supportedFeatures);

	return indices.isComplete() && extensionsSupported && swapChainAdequate &&
		   (supportedFeatures.samplerAnisotropy != 0u);
}

void VkEngineDevice::populateDebugMessengerCreateInfo(
	VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
					   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
					   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = debugCallback,
		.pUserData = nullptr // Optional
	};
}

void VkEngineDevice::setupDebugMessenger()
{

	if(!enableValidationLayers)
	{
		return;
	}

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);
	if(CreateDebugUtilsMessengerEXT(pInstance, &createInfo, nullptr, &pDebugMessenger) !=
	   VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

bool VkEngineDevice::checkValidationLayerSupport() const
{
	uint32_t layerCount = 0;
	if(const vk::Result r = vk::enumerateInstanceLayerProperties(&layerCount, nullptr);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to enumerate instance layer properties!");
	}

	auto* availableLayers = new vk::LayerProperties[layerCount];
	if(const vk::Result r = vk::enumerateInstanceLayerProperties(&layerCount, availableLayers);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to enumerate instance layer properties!");
	}

	for(const char* layerName : mValidationLayers)
	{
		bool layerFound = false;

		for(uint32_t j = 0; j < layerCount; ++j)
		{
			if(strcmp(layerName, availableLayers[j].layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if(!layerFound)
		{
			return false;
		}
	}

	delete[] availableLayers;

	return true;
}

std::vector<const char*> VkEngineDevice::getRequiredExtensions() const
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if(enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

#if __APPLE__
	extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

	return extensions;
}

void VkEngineDevice::hasGflwRequiredInstanceExtensions() const
{
	uint32_t extensionCount = 0;
	if(const vk::Result r =
		   vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to enumerate instance extension properties!");
	}

	auto* extensions = new vk::ExtensionProperties[extensionCount];

	if(const vk::Result r =
		   vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to enumerate instance extension properties!");
	}

	std::cout << "available extensions:" << '\n';
	std::unordered_set<std::string> available;
	for(uint32_t i = 0; i < extensionCount; ++i)
	{
		std::cout << "\t" << extensions[i].extensionName << '\n';
		available.insert(extensions[i].extensionName);
	}

	delete[] extensions;

	std::cout << "required extensions:" << '\n';
	for(const auto& required : getRequiredExtensions())
	{
		std::cout << "\t" << required << '\n';
		if(!available.contains(required))
		{
			throw std::runtime_error("Missing required glfw extension");
		}
	}
}

bool VkEngineDevice::checkDeviceExtensionSupport(const vk::PhysicalDevice device) const
{
	uint32_t extensionCount = 0;
	if(const vk::Result r =
		   device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to enumerate device extension properties!");
	}

	auto* availableExtensions = new vk::ExtensionProperties[extensionCount];

	if(const vk::Result r =
		   device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, availableExtensions);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to enumerate device extension properties!");
	}

	std::set<std::string> requiredExtensions(mDeviceExtensions.begin(), mDeviceExtensions.end());

	for(uint32_t i = 0; i < extensionCount; ++i)
	{
		requiredExtensions.erase(availableExtensions[i].extensionName);
	}

	delete[] availableExtensions;

	return requiredExtensions.empty();
}

QueueFamilyIndices VkEngineDevice::findQueueFamilies(const vk::PhysicalDevice device) const
{

	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	device.getQueueFamilyProperties(&queueFamilyCount, nullptr);

	auto* queueFamilies = new vk::QueueFamilyProperties[queueFamilyCount];
	device.getQueueFamilyProperties(&queueFamilyCount, queueFamilies);

	for(uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		if(queueFamilies[i].queueCount > 0 &&
		   queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			indices.mGraphicsFamily = i;
			indices.mGraphicsFamilyHasValue = true;
		}

		VkBool32 presentSupport = 0u;

		if(const vk::Result r = device.getSurfaceSupportKHR(i, pSurface, &presentSupport);
		   r != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to get surface support!");
		}

		if(queueFamilies[i].queueCount > 0 && presentSupport != 0u)
		{
			indices.mPresentFamily = i;
			indices.mPresentFamilyHasValue = true;
		}
		if(indices.isComplete())
		{
			break;
		}
	}

	delete[] queueFamilies;
	return indices;
}

SwapChainSupportDetails VkEngineDevice::querySwapChainSupport(const vk::PhysicalDevice device) const
{
	SwapChainSupportDetails details{};
	details.mCapabilities = device.getSurfaceCapabilitiesKHR(pSurface);

	uint32_t formatCount = 0;
	if(const vk::Result r = device.getSurfaceFormatsKHR(pSurface, &formatCount, nullptr);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to get surface formats!");
	}

	if(formatCount != 0)
	{
		details.mFormats.resize(formatCount);
		if(const vk::Result r =
			   device.getSurfaceFormatsKHR(pSurface, &formatCount, details.mFormats.data());
		   r != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to get surface formats!");
		}
	}

	uint32_t presentModeCount = 0;
	if(const vk::Result r = device.getSurfacePresentModesKHR(pSurface, &presentModeCount, nullptr);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to get surface present modes!");
	}

	if(presentModeCount != 0)
	{
		details.mPresentModes.resize(presentModeCount);
		if(const vk::Result r = device.getSurfacePresentModesKHR(
			   pSurface, &presentModeCount, details.mPresentModes.data());
		   r != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to get surface present modes!");
		}
	}
	return details;
}

vk::Format VkEngineDevice::findSupportedFormat(const std::vector<vk::Format>& candidates,
											   const vk::ImageTiling tiling,
											   const vk::FormatFeatureFlags features) const
{
	for(const vk::Format format : candidates)
	{
		vk::FormatProperties props;
		pPhysicalDevice.getFormatProperties(format, &props);

		if(tiling == vk::ImageTiling::eLinear &&
		   (props.linearTilingFeatures & features) == features)
		{
			return format;
		}

		if(tiling == vk::ImageTiling::eOptimal &&
		   (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}
	throw std::runtime_error("failed to find supported format!");
}

uint32_t VkEngineDevice::findMemoryType(const uint32_t typeFilter,
										const vk::MemoryPropertyFlags properties) const
{
	vk::PhysicalDeviceMemoryProperties memProperties;
	pPhysicalDevice.getMemoryProperties(&memProperties);
	for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if((typeFilter & 1 << i) != 0u &&
		   (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void VkEngineDevice::createBuffer(const vk::DeviceSize size,
								  const vk::BufferUsageFlags usage,
								  const vk::MemoryPropertyFlags properties,
								  vk::Buffer& buffer,
								  vk::DeviceMemory& bufferMemory) const
{

	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.setSize(size);
	bufferInfo.setUsage(usage);
	bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

	if(const vk::Result r = pDevice.createBuffer(&bufferInfo, nullptr, &buffer);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}

	vk::MemoryRequirements memRequirements;
	pDevice.getBufferMemoryRequirements(buffer, &memRequirements);

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.setAllocationSize(memRequirements.size);
	allocInfo.setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));

	if(const vk::Result r = pDevice.allocateMemory(&allocInfo, nullptr, &bufferMemory);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	pDevice.bindBufferMemory(buffer, bufferMemory, 0);
}

vk::CommandBuffer VkEngineDevice::beginSingleTimeCommands() const
{
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(pCommandPool);
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary);
	allocInfo.setCommandBufferCount(1);

	vk::CommandBuffer commandBuffer{};
	commandBuffer = pDevice.allocateCommandBuffers(allocInfo)[0];

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	if(const vk::Result r = commandBuffer.begin(&beginInfo); r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	return commandBuffer;
}

void VkEngineDevice::endSingleTimeCommands(vk::CommandBuffer commandBuffer) const
{
	vkEndCommandBuffer(commandBuffer);

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1);
	submitInfo.setPCommandBuffers(&commandBuffer);

	if(const vk::Result r = pGraphicsQueue.submit(1, &submitInfo, nullptr);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	pGraphicsQueue.waitIdle();
	pDevice.freeCommandBuffers(pCommandPool, 1, &commandBuffer);
}

void VkEngineDevice::copyBuffer(const vk::Buffer srcBuffer,
								const vk::Buffer dstBuffer,
								const vk::DeviceSize size) const
{
	const vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

	vk::BufferCopy copyRegion{};
	copyRegion.setSize(size);

	commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
	commandBuffer.end();
}

void VkEngineDevice::copyBufferToImage(const vk::Buffer buffer,
									   const vk::Image image,
									   const uint32_t width,
									   const uint32_t height,
									   const uint32_t layerCount) const
{
	const vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

	vk::BufferImageCopy region{};
	region.setBufferOffset(0);
	region.setBufferRowLength(0);
	region.setBufferImageHeight(0);
	region.setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, layerCount});

	commandBuffer.copyBufferToImage(
		buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

	commandBuffer.end();
}

void VkEngineDevice::createImageWithInfo(const vk::ImageCreateInfo& imageInfo,
										 const vk::MemoryPropertyFlags properties,
										 vk::Image& image,
										 vk::DeviceMemory& imageMemory) const
{
	if(const vk::Result r = pDevice.createImage(&imageInfo, nullptr, &image);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to create image!");
	}

	vk::MemoryRequirements memRequirements{};
	pDevice.getImageMemoryRequirements(image, &memRequirements);

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.setAllocationSize(memRequirements.size);
	allocInfo.setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));

	if(const vk::Result r = pDevice.allocateMemory(&allocInfo, nullptr, &imageMemory);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to allocate image memory!");
	}

	pDevice.bindImageMemory(image, imageMemory, 0);
}

} // namespace vke