#include "vkEngineSwapChain.hpp"

// std
#include <array>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace vke
{

VkEngineSwapChain::VkEngineSwapChain(VkEngineDevice& deviceRef, const VkExtent2D windowExtent)
	: device{deviceRef}
	  , windowExtent{windowExtent}
{
	init();
}

void VkEngineSwapChain::init()
{
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDepthResources();
	createFramebuffers();
	createSyncObjects();
}

VkEngineSwapChain::~VkEngineSwapChain()
{
	for(size_t i = 0; i < imageCount(); ++i)
	{
		vkDestroyImageView(device.device(), ppSwapChainImageViews[i], nullptr);
	}

	ppSwapChainImageViews = nullptr;

	if(swapChain != nullptr)
	{
		vkDestroySwapchainKHR(device.device(), swapChain, nullptr);
		swapChain = nullptr;
	}

	for(size_t i = 0; i < depthImages.size(); ++i)
	{
		vkDestroyImageView(device.device(), ppDepthImageViews[i], nullptr);
		vkDestroyImage(device.device(), depthImages[i], nullptr);
		vkFreeMemory(device.device(), ppDepthImageMemorys[i], nullptr);
	}

	delete[] ppDepthImageViews;
	delete[] ppDepthImageMemorys;

	for(size_t i = 0; i < imageCount(); ++i)
	{
		vkDestroyFramebuffer(device.device(), ppSwapChainFramebuffers[i], nullptr);
	}

	delete[] ppSwapChainFramebuffers;

	vkDestroyRenderPass(device.device(), pRenderPass, nullptr);

	// cleanup synchronization objects
	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(device.device(), ppRenderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device.device(), ppImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device.device(), ppInFlightFences[i], nullptr);
	}

	delete[] ppImageAvailableSemaphores;
	delete[] ppRenderFinishedSemaphores;
	delete[] ppInFlightFences;
}

VkResult VkEngineSwapChain::acquireNextImage(uint32_t* imageIndex) const
{
	vkWaitForFences(device.device(),
	                1,
	                &ppInFlightFences[currentFrame],
	                VK_TRUE,
	                std::numeric_limits <uint64_t>::max());

	const VkResult result = vkAcquireNextImageKHR(
		device.device(),
		swapChain,
		std::numeric_limits <uint64_t>::max(),
		ppImageAvailableSemaphores[currentFrame],
		// must be a not signaled semaphore
		VK_NULL_HANDLE,
		imageIndex);

	return result;
}

VkResult VkEngineSwapChain::submitCommandBuffers(const VkCommandBuffer* buffers,
                                                 const uint32_t* imageIndex)
{
	if(ppImagesInFlight[*imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(device.device(), 1, &ppImagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
	}

	ppImagesInFlight[*imageIndex] = ppInFlightFences[currentFrame];

	const std::array waitSemaphores = {ppImageAvailableSemaphores[currentFrame]};
	const std::array signalSemaphores = {ppRenderFinishedSemaphores[currentFrame]};
	constexpr std::array <VkPipelineStageFlags, 1> waitStages = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	const VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = waitSemaphores.data(),
		.pWaitDstStageMask = waitStages.data(),

		.commandBufferCount = 1,
		.pCommandBuffers = buffers,

		.signalSemaphoreCount = 1,
		.pSignalSemaphores = signalSemaphores.data(),
	};

	vkResetFences(device.device(), 1, &ppInFlightFences[currentFrame]);

	if(vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, ppInFlightFences[currentFrame]) !=
	   VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	const std::array swapChains = {swapChain};

	const VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = signalSemaphores.data(),
		.swapchainCount = 1,
		.pSwapchains = swapChains.data(),
		.pImageIndices = imageIndex,
	};

	const auto result = vkQueuePresentKHR(device.presentQueue(), &presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	return result;
}

void VkEngineSwapChain::createSwapChain()
{

	const VkPresentModeKHR presentMode =
		chooseSwapPresentMode(device.getSwapChainSupport().mPresentModes);
	const VkExtent2D extent = chooseSwapExtent(device.getSwapChainSupport().mCapabilities);

	uint32_t imageCount = device.getSwapChainSupport().mCapabilities.minImageCount + 1;
	if(device.getSwapChainSupport().mCapabilities.maxImageCount > 0 &&
	   imageCount > device.getSwapChainSupport().mCapabilities.maxImageCount)
	{
		imageCount = device.getSwapChainSupport().mCapabilities.maxImageCount;
	}

	const std::array queueFamilyIndices = {device.findPhysicalQueueFamilies().mGraphicsFamily,
	                                       device.findPhysicalQueueFamilies().mPresentFamily};

	const VkSwapchainCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = device.surface(),
		.minImageCount = imageCount,
		.imageFormat = chooseSwapSurfaceFormat(device.getSwapChainSupport().mFormats).format,
		.imageColorSpace =
		chooseSwapSurfaceFormat(device.getSwapChainSupport().mFormats).colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = device.findPhysicalQueueFamilies().mGraphicsFamily ==
		                         device.findPhysicalQueueFamilies().mPresentFamily
			                         ? 0u
			                         : 2,
		.pQueueFamilyIndices = device.findPhysicalQueueFamilies().mGraphicsFamily ==
		                       device.findPhysicalQueueFamilies().mPresentFamily
			                       ? nullptr
			                       : queueFamilyIndices.data(),
		.preTransform = device.getSwapChainSupport().mCapabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE,
	};

	if(vkCreateSwapchainKHR(device.device(), &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	// we only specified a minimum number of images in the swap chain, so the implementation is
	// allowed to create a swap chain with more. That's why we'll first query the final number of
	// images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
	// retrieve the handles.
	vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device.device(), swapChain, &imageCount, swapChainImages.data());

	mSwapChainImageFormat = chooseSwapSurfaceFormat(device.getSwapChainSupport().mFormats).format;
	mSwapChainExtent = extent;
}

void VkEngineSwapChain::createImageViews()
{
	ppSwapChainImageViews = new VkImageView[imageCount()];

	for(size_t i = 0; i < imageCount(); i++)
	{

		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = swapChainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = mSwapChainImageFormat,
			.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.subresourceRange.baseMipLevel = 0,
			.subresourceRange.levelCount = 1,
			.subresourceRange.baseArrayLayer = 0,
			.subresourceRange.layerCount = 1,
		};

		if(vkCreateImageView(device.device(), &viewInfo, nullptr, &ppSwapChainImageViews[i]) !=
		   VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image view!");
		}
	}
}

void VkEngineSwapChain::createRenderPass()
{
	const VkAttachmentDescription depthAttachment{
		.format = findDepthFormat(),
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentReference depthAttachmentRef{
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	const VkAttachmentDescription colorAttachment{
		.format = getSwapChainImageFormat(),
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentReference colorAttachmentRef{
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass{
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachmentRef,
		.pDepthStencilAttachment = &depthAttachmentRef,
	};

	VkSubpassDependency dependency{
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.srcAccessMask = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.dstSubpass = 0,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		.dstAccessMask =
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	};

	const std::array attachments = {colorAttachment, depthAttachment};

	const VkRenderPassCreateInfo renderPassInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = static_cast <uint32_t>(attachments.size()),
		.pAttachments = attachments.data(),
		.subpassCount = 1,
		.dependencyCount = 1,
		.pSubpasses = &subpass,
		.pDependencies = &dependency,
	};

	if(vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &pRenderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void VkEngineSwapChain::createFramebuffers()
{
	ppSwapChainFramebuffers = new VkFramebuffer[imageCount()];
	for(size_t i = 0; i < imageCount(); i++)
	{

		std::array attachments = {ppSwapChainImageViews[i], ppDepthImageViews[i]};

		VkFramebufferCreateInfo framebufferInfo{
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = pRenderPass,
			.attachmentCount = static_cast <uint32_t>(attachments.size()),
			.pAttachments = attachments.data(),
			.width = getSwapChainExtent().width,
			.height = getSwapChainExtent().height,
			.layers = 1,
		};

		if(vkCreateFramebuffer(
			   device.device(),
			   &framebufferInfo,
			   nullptr,
			   &ppSwapChainFramebuffers[i]) !=
		   VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void VkEngineSwapChain::createDepthResources()
{

	depthImages.resize(imageCount());
	ppDepthImageMemorys = new VkDeviceMemory[imageCount()];
	ppDepthImageViews = new VkImageView[imageCount()];

	for(size_t i = 0; i < depthImages.size(); i++)
	{
		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = findDepthFormat(),
			.extent = {.width = getSwapChainExtent().width,
			           .height = getSwapChainExtent().height,
			           .depth = 1},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};

		device.createImageWithInfo(
			imageInfo,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			depthImages[i],
			ppDepthImageMemorys[i]);

		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = depthImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = findDepthFormat(),
			.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.subresourceRange.baseMipLevel = 0,
			.subresourceRange.levelCount = 1,
			.subresourceRange.baseArrayLayer = 0,
			.subresourceRange.layerCount = 1,
		};

		if(vkCreateImageView(device.device(), &viewInfo, nullptr, &ppDepthImageViews[i]) !=
		   VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image view!");
		}
	}
}

void VkEngineSwapChain::createSyncObjects()
{

	ppImageAvailableSemaphores = new VkSemaphore[MAX_FRAMES_IN_FLIGHT];
	ppRenderFinishedSemaphores = new VkSemaphore[MAX_FRAMES_IN_FLIGHT];
	ppInFlightFences = new VkFence[MAX_FRAMES_IN_FLIGHT];
	ppImagesInFlight = new VkFence[imageCount()];

	constexpr VkSemaphoreCreateInfo semaphoreInfo{
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	constexpr VkFenceCreateInfo fenceInfo{
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if(vkCreateSemaphore(
			   device.device(),
			   &semaphoreInfo,
			   nullptr,
			   &ppImageAvailableSemaphores[i]) !=
		   VK_SUCCESS ||
		   vkCreateSemaphore(
			   device.device(),
			   &semaphoreInfo,
			   nullptr,
			   &ppRenderFinishedSemaphores[i]) !=
		   VK_SUCCESS ||
		   vkCreateFence(device.device(), &fenceInfo, nullptr, &ppInFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

VkSurfaceFormatKHR
VkEngineSwapChain::chooseSwapSurfaceFormat(const std::vector <VkSurfaceFormatKHR>& availableFormats)
{
	for(const auto& availableFormat : availableFormats)
	{
		if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
		   availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR
VkEngineSwapChain::chooseSwapPresentMode(
	const std::vector <VkPresentModeKHR>& availablePresentModes)
{
	for(const auto& availablePresentMode : availablePresentModes)
	{
		if(availablePresentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
		{
			std::cout << "Present mode: FIFO Relaxed" << '\n';
			return availablePresentMode;
		}
	}

	// for (const auto &availablePresentMode : availablePresentModes) {
	//   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
	//     std::cout << "Present mode: Immediate" << std::endl;
	//     return availablePresentMode;
	//   }
	// }

	std::cout << "Present mode: V-Sync" << '\n';
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VkEngineSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
{
	if(capabilities.currentExtent.width != std::numeric_limits <uint32_t>::max())
	{
		return capabilities.currentExtent;
	}

	VkExtent2D actualExtent = windowExtent;
	actualExtent.width = std::max(capabilities.minImageExtent.width,
	                              std::min(capabilities.maxImageExtent.width, actualExtent.width));
	actualExtent.height =
		std::max(capabilities.minImageExtent.height,
		         std::min(capabilities.maxImageExtent.height, actualExtent.height));

	return actualExtent;
}

VkFormat VkEngineSwapChain::findDepthFormat() const
{
	return device.findSupportedFormat(
		{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

} // namespace vke