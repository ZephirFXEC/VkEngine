#include "vkEngineSwapChain.hpp"

#include "utils/bufferUtils.hpp"

namespace vke {
VkEngineSwapChain::VkEngineSwapChain(const VkEngineDevice& deviceRef, const VkExtent2D windowExtent)
    : mDevice{deviceRef}, mWindowExtent{windowExtent} {
	init();
}

VkEngineSwapChain::VkEngineSwapChain(const VkEngineDevice& deviceRef, const VkExtent2D windowExtent,
                                     const std::shared_ptr<VkEngineSwapChain>& previous)
    : mDevice{deviceRef}, mWindowExtent{windowExtent}, pOldSwapChain{previous} {
	init();
	pOldSwapChain = nullptr;
}

void VkEngineSwapChain::init() {
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDepthResources();
	createFramebuffers();
	createCommandPools();
	createSyncObjects();
}

VkEngineSwapChain::~VkEngineSwapChain() {
	// vkDeviceWaitIdle(pDevice);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		vkDestroyCommandPool(mDevice.getDevice(), mFrameData.at(i).pCommandPool, nullptr);
	}

	for (size_t i = 0; i < imageCount(); ++i) {
		vkDestroyImageView(mDevice.getDevice(), mSwapChainImages.ppImageViews[i], nullptr);
	}

	if (pSwapChain != nullptr) {
		vkDestroySwapchainKHR(mDevice.getDevice(), pSwapChain, nullptr);
		pSwapChain = nullptr;
	}

	for (size_t i = 0; i < imageCount(); ++i) {
		vkDestroyImageView(mDevice.getDevice(), mDepthImages.ppImageViews[i], nullptr);
		vkDestroyImage(mDevice.getDevice(), mDepthImages.ppImages[i], nullptr);
#ifdef USE_VMA
		vmaFreeMemory(mDevice.getAllocator(), mDepthImages.ppImageMemorys[i]);
#else
		vkFreeMemory(mDevice.getDevice(), mDepthImages.ppImageMemorys[i], nullptr);
#endif

	}

	for (size_t i = 0; i < imageCount(); ++i) {
		vkDestroyFramebuffer(mDevice.getDevice(), ppSwapChainFramebuffers[i], nullptr);
	}

	delete[] ppSwapChainFramebuffers;

	vkDestroyRenderPass(mDevice.getDevice(), pRenderPass, nullptr);

	// cleanup synchronization objects
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		vkDestroySemaphore(mDevice.getDevice(), mSyncPrimitives.ppRenderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(mDevice.getDevice(), mSyncPrimitives.ppImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(mDevice.getDevice(), mSyncPrimitives.ppInFlightFences[i], nullptr);
	}

}

VkResult VkEngineSwapChain::acquireNextImage(uint32_t* imageIndex) const {
	VK_CHECK(vkWaitForFences(mDevice.getDevice(), 1, &mSyncPrimitives.ppInFlightFences[mCurrentFrame], VK_TRUE, UINT64_MAX));

	return vkAcquireNextImageKHR(mDevice.getDevice(), pSwapChain, UINT64_MAX,
	                             mSyncPrimitives.ppImageAvailableSemaphores[mCurrentFrame],
	                             // must be a not signaled semaphore
	                             VK_NULL_HANDLE, imageIndex);
}

VkResult VkEngineSwapChain::submitCommandBuffers(const VkCommandBuffer* buffers, const uint32_t* imageIndex) {
	if (mSyncPrimitives.ppInFlightImages[*imageIndex] != VK_NULL_HANDLE) {
		VK_CHECK(vkWaitForFences(mDevice.getDevice(), 1, &mSyncPrimitives.ppInFlightImages[*imageIndex], VK_TRUE, UINT64_MAX));
	}

	mSyncPrimitives.ppInFlightImages[*imageIndex] = mSyncPrimitives.ppInFlightFences[mCurrentFrame];

	constexpr VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	const VkSubmitInfo submitInfo = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	                                 .waitSemaphoreCount = 1,
	                                 .pWaitSemaphores = &mSyncPrimitives.ppImageAvailableSemaphores[mCurrentFrame],
	                                 .pWaitDstStageMask = &waitStages,

	                                 .commandBufferCount = 1,
	                                 .pCommandBuffers = buffers,

	                                 .signalSemaphoreCount = 1,
	                                 .pSignalSemaphores = &mSyncPrimitives.ppRenderFinishedSemaphores[mCurrentFrame]};

	VK_CHECK(vkResetFences(mDevice.getDevice(), 1, &mSyncPrimitives.ppInFlightFences[mCurrentFrame]));

	VK_CHECK(vkQueueSubmit(mDevice.getGraphicsQueue(), 1, &submitInfo, mSyncPrimitives.ppInFlightFences[mCurrentFrame]));

	const VkPresentInfoKHR presentInfo{
	    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
	    .waitSemaphoreCount = 1,
	    .pWaitSemaphores = &mSyncPrimitives.ppRenderFinishedSemaphores[mCurrentFrame],
	    .swapchainCount = 1,
	    .pSwapchains = &pSwapChain,
	    .pImageIndices = imageIndex,
	};

	mCurrentFrame = (mCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	return vkQueuePresentKHR(mDevice.getPresentQueue(), &presentInfo);
}

void VkEngineSwapChain::createSwapChain() {
	const VkPresentModeKHR presentMode = chooseSwapPresentMode(mDevice.getSwapChainSupport().mPresentModes);
	const VkExtent2D extent = chooseSwapExtent(mDevice.getSwapChainSupport().mCapabilities);

	uint32_t imageCount = mDevice.getSwapChainSupport().mCapabilities.minImageCount + 1;

	if (mDevice.getSwapChainSupport().mCapabilities.maxImageCount > 0 &&
	    imageCount > mDevice.getSwapChainSupport().mCapabilities.maxImageCount) {
		imageCount = mDevice.getSwapChainSupport().mCapabilities.maxImageCount;
	}

	QueueFamilyIndices indices{};
	if (mDevice.findPhysicalQueueFamilies().isComplete()) {
		indices = mDevice.findPhysicalQueueFamilies();
	}

	const std::array queueFamilyIndices = {indices.mGraphicsFamily.value(), indices.mPresentFamily.value()};

	const VkSwapchainCreateInfoKHR createInfo{
	    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
	    .surface = mDevice.getSurface(),
	    .minImageCount = imageCount,
	    .imageFormat = chooseSwapSurfaceFormat(mDevice.getSwapChainSupport().mFormats).format,
	    .imageColorSpace = chooseSwapSurfaceFormat(mDevice.getSwapChainSupport().mFormats).colorSpace,
	    .imageExtent = extent,
	    .imageArrayLayers = 1,
	    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
	    .queueFamilyIndexCount = indices.mGraphicsFamily.value() == indices.mPresentFamily.value() ? 0u : 2,
	    .pQueueFamilyIndices =
	        indices.mGraphicsFamily.value() == indices.mPresentFamily.value() ? nullptr : queueFamilyIndices.data(),
	    .preTransform = mDevice.getSwapChainSupport().mCapabilities.currentTransform,
	    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
	    .presentMode = presentMode,
	    .clipped = VK_TRUE,
	    .oldSwapchain = pOldSwapChain == nullptr ? VK_NULL_HANDLE : pOldSwapChain->pSwapChain,
	};

	VK_CHECK(vkCreateSwapchainKHR(mDevice.getDevice(), &createInfo, nullptr, &pSwapChain));

	// we only specified a minimum number of images in the swap chain, so the
	// implementation is allowed to create a swap chain with more. That's why
	// we'll first query the final number of images with
	// vkGetSwapchainImagesKHR, then resize the container and finally call it
	// again to retrieve the handles.
	VK_CHECK(vkGetSwapchainImagesKHR(mDevice.getDevice(), pSwapChain, &mSwapChainImageCount, nullptr));

	mSwapChainImages.ppImages = new VkImage[imageCount]{};
	VK_CHECK(vkGetSwapchainImagesKHR(mDevice.getDevice(), pSwapChain, &mSwapChainImageCount, mSwapChainImages.ppImages));

	mSwapChainImageFormat = chooseSwapSurfaceFormat(mDevice.getSwapChainSupport().mFormats).format;
	mSwapChainExtent = extent;
}

void VkEngineSwapChain::createImageViews() {
	mSwapChainImages.ppImageViews = new VkImageView[imageCount()]{};

	for (size_t i = 0; i < imageCount(); i++) {
		VkImageViewCreateInfo viewInfo{
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

		VK_CHECK(vkCreateImageView(mDevice.getDevice(), &viewInfo, nullptr, &mSwapChainImages.ppImageViews[i]));
	}
}

void VkEngineSwapChain::createRenderPass() {
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
	    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
	    .dstSubpass = 0,
	    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
	    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
	};

	const std::array attachments = {colorAttachment, depthAttachment};

	const VkRenderPassCreateInfo renderPassInfo{
	    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
	    .attachmentCount = static_cast<uint32_t>(attachments.size()),
	    .pAttachments = attachments.data(),
	    .subpassCount = 1,
	    .dependencyCount = 1,
	    .pSubpasses = &subpass,
	    .pDependencies = &dependency,
	};

	VK_CHECK(vkCreateRenderPass(mDevice.getDevice(), &renderPassInfo, nullptr, &pRenderPass));
}

void VkEngineSwapChain::createFramebuffers() {
	ppSwapChainFramebuffers = new VkFramebuffer[imageCount()]{};

	for (size_t i = 0; i < imageCount(); i++) {
		std::array attachments = {mSwapChainImages.ppImageViews[i], mDepthImages.ppImageViews[i]};

		VkFramebufferCreateInfo framebufferInfo{
		    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		    .renderPass = pRenderPass,
		    .attachmentCount = static_cast<uint32_t>(attachments.size()),
		    .pAttachments = attachments.data(),
		    .width = getSwapChainExtent().width,
		    .height = getSwapChainExtent().height,
		    .layers = 1,
		};

		VK_CHECK(vkCreateFramebuffer(mDevice.getDevice(), &framebufferInfo, nullptr, &ppSwapChainFramebuffers[i]));
	}
}

void VkEngineSwapChain::createDepthResources() {
	mDepthImages.ppImages = new VkImage[imageCount()]{};
	mDepthImages.ppImageMemorys = new Alloc[imageCount()]{};
	mDepthImages.ppImageViews = new VkImageView[imageCount()]{};


	for (size_t i = 0; i < imageCount(); i++) {
		VkImageCreateInfo imageInfo{
		    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		    .imageType = VK_IMAGE_TYPE_2D,
		    .format = findDepthFormat(),
		    .extent = {.width = getSwapChainExtent().width, .height = getSwapChainExtent().height, .depth = 1},
		    .mipLevels = 1,
		    .arrayLayers = 1,
		    .samples = VK_SAMPLE_COUNT_1_BIT,
		    .tiling = VK_IMAGE_TILING_OPTIMAL,
		    .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};

		// constexpr VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
		createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mDepthImages.ppImages[i],
		                    mDepthImages.ppImageMemorys[i]);

		VkImageViewCreateInfo viewInfo{
		    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		    .image = mDepthImages.ppImages[i],
		    .viewType = VK_IMAGE_VIEW_TYPE_2D,
		    .format = findDepthFormat(),
		    .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
		                         .baseMipLevel = 0,
		                         .levelCount = 1,
		                         .baseArrayLayer = 0,
		                         .layerCount = 1},

		};

		VK_CHECK(vkCreateImageView(mDevice.getDevice(), &viewInfo, nullptr, &mDepthImages.ppImageViews[i]));
	}
}

void VkEngineSwapChain::createCommandPools() {
	const VkCommandPoolCreateInfo poolInfo = {
	    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
	    .queueFamilyIndex = mDevice.findPhysicalQueueFamilies().mGraphicsFamily.value(),

	};

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VK_CHECK(vkCreateCommandPool(mDevice.getDevice(), &poolInfo, nullptr, &mFrameData.at(i).pCommandPool));

		// allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo{
		    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		    .pNext = nullptr,
		    .commandPool = mFrameData.at(i).pCommandPool,
		    .commandBufferCount = 1,
		    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		};

		VK_CHECK(vkAllocateCommandBuffers(mDevice.getDevice(), &cmdAllocInfo, &mFrameData.at(i).pCommandBuffer));
	}
}

void VkEngineSwapChain::createSyncObjects() {
	mSyncPrimitives.ppImageAvailableSemaphores = new VkSemaphore[MAX_FRAMES_IN_FLIGHT]{};
	mSyncPrimitives.ppRenderFinishedSemaphores = new VkSemaphore[MAX_FRAMES_IN_FLIGHT]{};
	mSyncPrimitives.ppInFlightFences = new VkFence[MAX_FRAMES_IN_FLIGHT]{};

	mSyncPrimitives.ppInFlightImages = new VkFence[imageCount()]{};
	memset(mSyncPrimitives.ppInFlightImages, 0, sizeof(VkFence) * imageCount());

	constexpr VkSemaphoreCreateInfo semaphoreInfo{
	    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};

	constexpr VkFenceCreateInfo fenceInfo{
	    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
	    .flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {

		VK_CHECK(vkCreateSemaphore(mDevice.getDevice(), &semaphoreInfo, nullptr, &mSyncPrimitives.ppImageAvailableSemaphores[i]));
		VK_CHECK(vkCreateSemaphore(mDevice.getDevice(), &semaphoreInfo, nullptr, &mSyncPrimitives.ppRenderFinishedSemaphores[i]));
		VK_CHECK(vkCreateFence(mDevice.getDevice(), &fenceInfo, nullptr, &mSyncPrimitives.ppInFlightFences[i]));
	}
}

void VkEngineSwapChain::createBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage,
                                     const VkMemoryPropertyFlags properties, VkBuffer& buffer,
                                     Alloc& bufferMemory) const {
	const VkBufferCreateInfo bufferInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	                                    .size = size,
	                                    .usage = usage,
	                                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

#ifdef USE_VMA
	constexpr VmaAllocationCreateInfo allocInfo{
	    .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
	    .usage = VMA_MEMORY_USAGE_AUTO,
	};

	VK_CHECK(vmaCreateBuffer(mDevice.getAllocator(), &bufferInfo, &allocInfo, &buffer, &bufferMemory, nullptr));

	vmaDestroyBuffer(mDevice.getAllocator(), buffer, bufferMemory);
#else
	VK_CHECK(vkCreateBuffer(pDevice, &bufferInfo, nullptr, &buffer));

	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(pDevice, buffer, &memRequirements);

	const VkMemoryAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	                                     .allocationSize = memRequirements.size,
	                                     .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)};

	VK_CHECK(vkAllocateMemory(pDevice, &allocInfo, nullptr, &bufferMemory));

	VK_CHECK(vkBindBufferMemory(pDevice, buffer, bufferMemory, 0));

	// clear memory
	vkDestroyBuffer(pDevice, buffer, nullptr);
	vkFreeMemory(pDevice, bufferMemory, nullptr);
#endif
}

void VkEngineSwapChain::copyBufferToImage(const VkBuffer* const buffer, const VkImage* const image,
                                          const uint32_t width, const uint32_t height, const uint32_t layerCount) {
	BufferUtils::beginSingleTimeCommands(mDevice.getDevice(), mFrameData.at(mCurrentFrame));

	const VkBufferImageCopy region{.bufferOffset = 0,
	                               .bufferRowLength = 0,
	                               .bufferImageHeight = 0,

	                               .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	                                                    .mipLevel = 0,
	                                                    .baseArrayLayer = 0,
	                                                    .layerCount = layerCount},

	                               .imageOffset = {0, 0, 0},
	                               .imageExtent = {width, height, 1}};

	vkCmdCopyBufferToImage(mFrameData.at(mCurrentFrame).pCommandBuffer, *buffer, *image,
	                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	BufferUtils::endSingleTimeCommands(mDevice.getDevice(), mFrameData.at(mCurrentFrame), mDevice.getGraphicsQueue());
}

void VkEngineSwapChain::createImageWithInfo(const VkImageCreateInfo& imageInfo, const VkMemoryPropertyFlags properties,
                                            VkImage& image, Alloc& imageMemory) const {
#ifdef USE_VMA
	constexpr VmaAllocationCreateInfo rimg_allocinfo{
	    .usage = VMA_MEMORY_USAGE_GPU_ONLY,
	    .requiredFlags = static_cast<VkMemoryPropertyFlags>(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};

	vmaCreateImage(mDevice.getAllocator(), &imageInfo, &rimg_allocinfo, &image, &imageMemory, nullptr);

#else

	VK_CHECK(vkCreateImage(pDevice, &imageInfo, nullptr, &image));

	VkMemoryRequirements memRequirements{};
	vkGetImageMemoryRequirements(pDevice, image, &memRequirements);

	const VkMemoryAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	                                     .allocationSize = memRequirements.size,
	                                     .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)};

	VK_CHECK(vkAllocateMemory(pDevice, &allocInfo, nullptr, &imageMemory));

	VK_CHECK(vkBindImageMemory(pDevice, image, imageMemory, 0));
#endif
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
		                                         return availablePresentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR;
	                                         });
	    it != availablePresentModes.end()) {
		fmt::println("Present mode: FIFO Relaxed");
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
	return mDevice.findSupportedFormat(
	    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL,
	    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
}  // namespace vke