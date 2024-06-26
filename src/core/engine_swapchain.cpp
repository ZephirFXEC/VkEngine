#include "engine_swapchain.hpp"

#include <vulkan/vulkan_core.h>

#include "engine_device.hpp"
#include "utils/logger.hpp"
#include "utils/memory.hpp"


namespace vke {
VkEngineSwapChain::VkEngineSwapChain(std::shared_ptr<VkEngineDevice> device, const VkExtent2D windowExtent)
    : mDevice{std::move(device)}, mWindowExtent{windowExtent} {
	init();
}

VkEngineSwapChain::VkEngineSwapChain(std::shared_ptr<VkEngineDevice> device, const VkExtent2D windowExtent,
                                     const std::shared_ptr<VkEngineSwapChain>& previous)
    : mDevice{std::move(device)}, mWindowExtent{windowExtent}, pOldSwapChain{previous} {
	init();
	pOldSwapChain = nullptr;
}

void VkEngineSwapChain::init() {
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDepthResources();
	createFramebuffers();
	createSyncObjects();
}


VkEngineSwapChain::~VkEngineSwapChain() {
	VKINFO("Destroyed swapchain");

	mDeletionQueue.flush();
}

VkResult VkEngineSwapChain::acquireNextImage(u32* imageIndex) const {
	VK_CHECK(vkWaitForFences(mDevice->getDevice(), 1, &mSyncPrimitives.ppInFlightFences[mCurrentFrame], VK_TRUE,
	                         UINT64_MAX));


	return vkAcquireNextImageKHR(mDevice->getDevice(), pSwapChain, UINT64_MAX,
	                             mSyncPrimitives.ppImageAvailableSemaphores[mCurrentFrame],
	                             // must be a not signaled semaphore
	                             VK_NULL_HANDLE, imageIndex);
}

VkResult VkEngineSwapChain::submitCommandBuffers(const VkCommandBuffer* buffers, const u32* imageIndex) {
	if (mSyncPrimitives.ppInFlightImages[*imageIndex] != VK_NULL_HANDLE) {
		VK_CHECK(vkWaitForFences(mDevice->getDevice(), 1, &mSyncPrimitives.ppInFlightImages[*imageIndex], VK_TRUE,
		                         UINT64_MAX));
	}

	mSyncPrimitives.ppInFlightImages[*imageIndex] = mSyncPrimitives.ppInFlightFences[mCurrentFrame];

	constexpr std::array<VkPipelineStageFlags, 1> waitStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	const VkSubmitInfo submitInfo = {
	    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	    .waitSemaphoreCount = 1,
	    .pWaitSemaphores = &mSyncPrimitives.ppImageAvailableSemaphores[mCurrentFrame],
	    .pWaitDstStageMask = waitStages.data(),

	    .commandBufferCount = 1,
	    .pCommandBuffers = buffers,

	    .signalSemaphoreCount = 1,
	    .pSignalSemaphores = &mSyncPrimitives.ppRenderFinishedSemaphores[mCurrentFrame]};

	VK_CHECK(vkResetFences(mDevice->getDevice(), 1, &mSyncPrimitives.ppInFlightFences[mCurrentFrame]));

	VK_CHECK(
	    vkQueueSubmit(mDevice->getGraphicsQueue(), 1, &submitInfo, mSyncPrimitives.ppInFlightFences[mCurrentFrame]));

	const VkPresentInfoKHR presentInfo{
	    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
	    .waitSemaphoreCount = 1,
	    .pWaitSemaphores = &mSyncPrimitives.ppRenderFinishedSemaphores[mCurrentFrame],
	    .swapchainCount = 1,
	    .pSwapchains = &pSwapChain,
	    .pImageIndices = imageIndex,
	};

	mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	return vkQueuePresentKHR(mDevice->getPresentQueue(), &presentInfo);
}

void VkEngineSwapChain::createSwapChain() {
	const VkPresentModeKHR presentMode = chooseSwapPresentMode(mDevice->getSwapChainSupport().mPresentModes);
	const VkExtent2D extent = chooseSwapExtent(mDevice->getSwapChainSupport().mCapabilities);

	u32 imageCount = mDevice->getSwapChainSupport().mCapabilities.minImageCount + 1;

	if (mDevice->getSwapChainSupport().mCapabilities.maxImageCount > 0 &&
	    imageCount > mDevice->getSwapChainSupport().mCapabilities.maxImageCount) {
		imageCount = mDevice->getSwapChainSupport().mCapabilities.maxImageCount;
	}

	QueueFamilyIndices indices{};
	if (mDevice->findPhysicalQueueFamilies().isComplete()) {
		indices = mDevice->findPhysicalQueueFamilies();
	}

	const std::array queueFamilyIndices = {indices.mGraphicsFamily.value(), indices.mPresentFamily.value()};

	const VkSwapchainCreateInfoKHR createInfo{
	    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
	    .surface = mDevice->getSurface(),
	    .minImageCount = imageCount,
	    .imageFormat = chooseSwapSurfaceFormat(mDevice->getSwapChainSupport().mFormats).format,
	    .imageColorSpace = chooseSwapSurfaceFormat(mDevice->getSwapChainSupport().mFormats).colorSpace,
	    .imageExtent = extent,
	    .imageArrayLayers = 1,
	    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
	    .queueFamilyIndexCount = indices.mGraphicsFamily.value() == indices.mPresentFamily.value() ? 0u : 2,
	    .pQueueFamilyIndices =
	        indices.mGraphicsFamily.value() == indices.mPresentFamily.value() ? nullptr : queueFamilyIndices.data(),
	    .preTransform = mDevice->getSwapChainSupport().mCapabilities.currentTransform,
	    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
	    .presentMode = presentMode,
	    .clipped = VK_TRUE,
	    .oldSwapchain = pOldSwapChain == nullptr ? VK_NULL_HANDLE : pOldSwapChain->pSwapChain,
	};

	VK_CHECK(vkCreateSwapchainKHR(mDevice->getDevice(), &createInfo, nullptr, &pSwapChain));
	mDeletionQueue.push_function([this]() { vkDestroySwapchainKHR(mDevice->getDevice(), pSwapChain, nullptr); });

	// we only specified a minimum number of images in the swap chain, so the
	// implementation is allowed to create a swap chain with more. That's why
	// we'll first query the final number of images with
	// vkGetSwapchainImagesKHR, then resize the container and finally call it
	// again to retrieve the handles.
	VK_CHECK(vkGetSwapchainImagesKHR(mDevice->getDevice(), pSwapChain, &mSwapChainImageCount, nullptr));


	mSwapChainImages.ppImages = Memory::allocMemory<VkImage>(getImageCount(), MEMORY_TAG_VULKAN);

	VK_CHECK(
	    vkGetSwapchainImagesKHR(mDevice->getDevice(), pSwapChain, &mSwapChainImageCount, mSwapChainImages.ppImages));

	mSwapChainImageFormat = chooseSwapSurfaceFormat(mDevice->getSwapChainSupport().mFormats).format;
	mSwapChainExtent = extent;
}

void VkEngineSwapChain::createImageViews() {
	mSwapChainImages.ppImageViews = Memory::allocMemory<VkImageView>(getImageCount(), MEMORY_TAG_VULKAN);

	for (size_t i = 0; i < getImageCount(); ++i) {
		const VkImageViewCreateInfo viewInfo{
		    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		    .image = mSwapChainImages.ppImages[i],
		    .viewType = VK_IMAGE_VIEW_TYPE_2D,
		    .format = mSwapChainImageFormat,
		    .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		                         .baseMipLevel = 0,
		                         .levelCount = 1,
		                         .baseArrayLayer = 0,
		                         .layerCount = 1},
		};

		VK_CHECK(vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &mSwapChainImages.ppImageViews[i]));
	}
}

void VkEngineSwapChain::createRenderPass() {
	constexpr VkAttachmentDescription depthAttachment{
	    .format = VK_FORMAT_D24_UNORM_S8_UINT,
	    .samples = VK_SAMPLE_COUNT_1_BIT,
	    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
	    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	    .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	const VkAttachmentDescription colorAttachment{
	    .format = getSwapChainImageFormat(),
	    .samples = VK_SAMPLE_COUNT_1_BIT,
	    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
	    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
	    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};


	static constexpr VkAttachmentReference depthAttachmentRef{
	    .attachment = 1,
	    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};


	static constexpr VkAttachmentReference colorAttachmentRef{
	    .attachment = 0,
	    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	constexpr VkSubpassDescription subpass{
	    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
	    .colorAttachmentCount = 1,
	    .pColorAttachments = &colorAttachmentRef,
	    .pDepthStencilAttachment = &depthAttachmentRef,
	};

	constexpr VkSubpassDependency dependency{
	    .srcSubpass = VK_SUBPASS_EXTERNAL,
	    .dstSubpass = 0,
	    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
	    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
	    .srcAccessMask = 0,
	    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	};

	const std::array attachments = {colorAttachment, depthAttachment};

	const VkRenderPassCreateInfo renderPassInfo{
	    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
	    .attachmentCount = static_cast<u32>(attachments.size()),
	    .pAttachments = attachments.data(),
	    .subpassCount = 1,
	    .pSubpasses = &subpass,
	    .dependencyCount = 1,
	    .pDependencies = &dependency,
	};

	VK_CHECK(vkCreateRenderPass(mDevice->getDevice(), &renderPassInfo, nullptr, &pRenderPass));
	mDeletionQueue.push_function([this]() { vkDestroyRenderPass(mDevice->getDevice(), pRenderPass, nullptr); });
}

void VkEngineSwapChain::createFramebuffers() {
	ppSwapChainFramebuffers = Memory::allocMemory<VkFramebuffer>(getImageCount(), MEMORY_TAG_VULKAN);

	for (size_t i = 0; i < getImageCount(); ++i) {
		std::array attachments = {mSwapChainImages.ppImageViews[i], mDepthImages.ppImageViews[i]};

		const VkFramebufferCreateInfo framebufferInfo{
		    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		    .renderPass = pRenderPass,
		    .attachmentCount = static_cast<u32>(attachments.size()),
		    .pAttachments = attachments.data(),
		    .width = getSwapChainExtent().width,
		    .height = getSwapChainExtent().height,
		    .layers = 1,
		};

		VK_CHECK(vkCreateFramebuffer(mDevice->getDevice(), &framebufferInfo, nullptr, &ppSwapChainFramebuffers[i]));
	}
}

void VkEngineSwapChain::createDepthResources() {
	mDepthImages.ppImages = Memory::allocMemory<VkImage>(getImageCount(), MEMORY_TAG_VULKAN);
	mDepthImages.ppImageMemorys = Memory::allocMemory<VmaAllocation>(getImageCount(), MEMORY_TAG_VULKAN);
	mDepthImages.ppImageViews = Memory::allocMemory<VkImageView>(getImageCount(), MEMORY_TAG_VULKAN);

	mDepthFormat = findDepthFormat();

	for (size_t i = 0; i < getImageCount(); i++) {
		const VkImageCreateInfo imageInfo{
		    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		    .imageType = VK_IMAGE_TYPE_2D,
		    .format = mDepthFormat,
		    .extent = {.width = getSwapChainExtent().width, .height = getSwapChainExtent().height, .depth = 1},
		    .mipLevels = 1,
		    .arrayLayers = 1,
		    .samples = VK_SAMPLE_COUNT_1_BIT,
		    .tiling = VK_IMAGE_TILING_OPTIMAL,
		    .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};

		mDevice->createImageWithInfo(imageInfo, mDepthImages.ppImages[i], mDepthImages.ppImageMemorys[i]);

		const VkImageViewCreateInfo viewInfo{
		    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		    .image = mDepthImages.ppImages[i],
		    .viewType = VK_IMAGE_VIEW_TYPE_2D,
		    .format = mDepthFormat,
		    .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
		                         .baseMipLevel = 0,
		                         .levelCount = 1,
		                         .baseArrayLayer = 0,
		                         .layerCount = 1},
		};

		VK_CHECK(vkCreateImageView(mDevice->getDevice(), &viewInfo, nullptr, &mDepthImages.ppImageViews[i]));
		mDeletionQueue.push_function([this, i]() {
			vkDestroyImageView(mDevice->getDevice(), mDepthImages.ppImageViews[i], nullptr);
			vmaDestroyImage(mDevice->getAllocator(), mDepthImages.ppImages[i], mDepthImages.ppImageMemorys[i]);
		});
	}
}


void VkEngineSwapChain::createSyncObjects() {
	mSyncPrimitives.ppInFlightImages = Memory::allocMemory<VkFence>(getImageCount(), MEMORY_TAG_VULKAN);

	constexpr VkSemaphoreCreateInfo semaphoreInfo{
	    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	constexpr VkFenceCreateInfo fenceInfo{
	    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
	    .flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		VK_CHECK(vkCreateSemaphore(mDevice->getDevice(), &semaphoreInfo, nullptr,
		                           &mSyncPrimitives.ppImageAvailableSemaphores[i]));

		mDeletionQueue.push_function([this, i]() {
			vkDestroySemaphore(mDevice->getDevice(), mSyncPrimitives.ppImageAvailableSemaphores[i], nullptr);
		});

		VK_CHECK(vkCreateSemaphore(mDevice->getDevice(), &semaphoreInfo, nullptr,
		                           &mSyncPrimitives.ppRenderFinishedSemaphores[i]));
		mDeletionQueue.push_function([this, i]() {
			vkDestroySemaphore(mDevice->getDevice(), mSyncPrimitives.ppRenderFinishedSemaphores[i], nullptr);
		});

		VK_CHECK(vkCreateFence(mDevice->getDevice(), &fenceInfo, nullptr, &mSyncPrimitives.ppInFlightFences[i]));

		mDeletionQueue.push_function([this, i]() {
			vkDestroyFence(mDevice->getDevice(), mSyncPrimitives.ppInFlightFences[i], nullptr);
		});
	}
}

VkSurfaceFormatKHR VkEngineSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	if (const auto it = std::ranges::find_if(availableFormats.begin(), availableFormats.end(),
	                                         [](const VkSurfaceFormatKHR& availableFormat) {
		                                         return availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
		                                                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	                                         });
	    it != availableFormats.end()) {
		return *it;
	}

	return availableFormats[0];
}

VkPresentModeKHR VkEngineSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	if (const auto it = std::ranges::find_if(availablePresentModes.begin(), availablePresentModes.end(),
	                                         [](const VkPresentModeKHR& availablePresentMode) {
		                                         return availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR;
	                                         });
	    it != availablePresentModes.end()) {
		fmt::println("Present mode: Immediate");
		return *it;
	}

	fmt::println("Present mode: V-Sync");
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VkEngineSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}

	VkExtent2D actualExtent = mWindowExtent;
	actualExtent.width =
	    std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
	actualExtent.height =
	    std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

	return actualExtent;
}

VkFormat VkEngineSwapChain::findDepthFormat() const {
	// TODO: Check for different formats
	return mDevice->findSupportedFormat({VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL,
	                                   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
}  // namespace vke