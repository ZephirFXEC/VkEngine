#include "vkEngineSwapChain.hpp"

// std
#include <array>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace vke
{

VkEngineSwapChain::VkEngineSwapChain(VkEngineDevice& deviceRef, vk::Extent2D extent)
	: device{deviceRef}
	, windowExtent{extent}
{
	init();
}

VkEngineSwapChain::VkEngineSwapChain(VkEngineDevice& deviceRef,
									 vk::Extent2D extent,
									 std::shared_ptr<VkEngineSwapChain> previous)
	: device{deviceRef}
	, windowExtent{extent}
	, oldSwapChain{std::move(std::move(previous))}
{
	init();
	oldSwapChain = nullptr;
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
		device.device().destroyImageView(ppSwapChainImageViews[i]);
	}

	ppSwapChainImageViews.clear();

	if(swapChain != nullptr)
	{
		device.device().destroySwapchainKHR(swapChain);
		swapChain = nullptr;
	}

	for(size_t i = 0; i < depthImages.size(); ++i)
	{
		device.device().destroyImageView(ppDepthImageViews[i]);
		device.device().destroyImage(depthImages[i]);
		device.device().free(ppDepthImageMemorys[i]);
	}

	for(size_t i = 0; i < imageCount(); ++i)
	{
		device.device().destroyFramebuffer(ppSwapChainFramebuffers[i]);
	}

	device.device().destroyRenderPass(pRenderPass);

	// cleanup synchronization objects
	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		device.device().destroySemaphore(ppImageAvailableSemaphores[i]);
		device.device().destroySemaphore(ppRenderFinishedSemaphores[i]);
		device.device().destroyFence(ppInFlightFences[i]);
	}
}

vk::Result VkEngineSwapChain::acquireNextImage(uint32_t* imageIndex) const
{

	if(const vk::Result r = device.device().waitForFences(
		   1, &ppInFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to wait for fences!");
	}

	const vk::Result result =
		device.device().acquireNextImageKHR(swapChain,
											std::numeric_limits<uint64_t>::max(),
											ppImageAvailableSemaphores[currentFrame],
											VK_NULL_HANDLE,
											imageIndex);

	return result;
}

vk::Result VkEngineSwapChain::submitCommandBuffers(const vk::CommandBuffer* buffers,
												   const uint32_t* imageIndex)
{
	if(ppImagesInFlight[*imageIndex] != VK_NULL_HANDLE)
	{
		if(const vk::Result r = device.device().waitForFences(
			   1, &ppImagesInFlight[*imageIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
		   r != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to wait for fences!");
		}
	}

	ppImagesInFlight[*imageIndex] = ppInFlightFences[currentFrame];

	const std::array waitSemaphores = {ppImageAvailableSemaphores[currentFrame]};
	const std::array signalSemaphores = {ppRenderFinishedSemaphores[currentFrame]};
	constexpr std::array<vk::PipelineStageFlags, 1> waitStages = {
		vk::PipelineStageFlagBits::eColorAttachmentOutput};

	const vk::SubmitInfo submitInfo(
		1, waitSemaphores.data(), waitStages.data(), 1, buffers, 1, signalSemaphores.data());

	if(const vk::Result r = device.device().resetFences(1, &ppInFlightFences[currentFrame]);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to reset fences!");
	}

	if(const vk::Result r =
		   device.graphicsQueue().submit(1, &submitInfo, ppInFlightFences[currentFrame]);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	const std::array swapChains = {swapChain};

	const vk::PresentInfoKHR presentInfo(
		1, signalSemaphores.data(), 1, swapChains.data(), imageIndex, nullptr);

	const vk::Result result = device.presentQueue().presentKHR(&presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	return result;
}

void VkEngineSwapChain::createSwapChain()
{

	const SwapChainSupportDetails swapChainSupport = device.getSwapChainSupport();

	const vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.mFormats);
	const vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.mPresentModes);
	const vk::Extent2D extent = chooseSwapExtent(swapChainSupport.mCapabilities);

	uint32_t imageCount = swapChainSupport.mCapabilities.minImageCount + 1;
	if(swapChainSupport.mCapabilities.maxImageCount > 0 &&
	   imageCount > swapChainSupport.mCapabilities.maxImageCount)
	{
		imageCount = swapChainSupport.mCapabilities.maxImageCount;
	}

	const QueueFamilyIndices indices = device.findPhysicalQueueFamilies();
	const std::array queueFamilyIndices = {indices.mGraphicsFamily, indices.mPresentFamily};

	const vk::SwapchainCreateInfoKHR createInfo(
		vk::SwapchainCreateFlagsKHR{},
		device.surface(),
		imageCount,
		surfaceFormat.format,
		surfaceFormat.colorSpace,
		extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		indices.mGraphicsFamily == indices.mPresentFamily ? 0u : 2,
		indices.mGraphicsFamily == indices.mPresentFamily ? nullptr : queueFamilyIndices.data(),
		swapChainSupport.mCapabilities.currentTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		presentMode,
		VK_TRUE,
		oldSwapChain == nullptr ? VK_NULL_HANDLE : oldSwapChain->swapChain);

	if(const vk::Result r = device.device().createSwapchainKHR(&createInfo, nullptr, &swapChain);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	// we only specified a minimum number of images in the swap chain, so the implementation is
	// allowed to create a swap chain with more. That's why we'll first query the final number of
	// images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
	// retrieve the handles.
	if(const vk::Result r = device.device().getSwapchainImagesKHR(swapChain, &imageCount, nullptr);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to get swap chain images!");
	}

	swapChainImages.resize(imageCount);
	if(const vk::Result r =
		   device.device().getSwapchainImagesKHR(swapChain, &imageCount, swapChainImages.data());
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to get swap chain images!");
	}

	mSwapChainImageFormat = surfaceFormat.format;
	mSwapChainExtent = extent;
}

void VkEngineSwapChain::createImageViews()
{
	ppSwapChainImageViews.resize(imageCount());

	for(size_t i = 0; i < swapChainImages.size(); i++)
	{

		vk::ImageViewCreateInfo viewInfo(
			vk::ImageViewCreateFlags{},
			swapChainImages[i],
			vk::ImageViewType::e2D,
			mSwapChainImageFormat,
			vk::ComponentMapping{vk::ComponentSwizzle::eIdentity,
								 vk::ComponentSwizzle::eIdentity,
								 vk::ComponentSwizzle::eIdentity,
								 vk::ComponentSwizzle::eIdentity},
			vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

		if(const vk::Result r =
			   device.device().createImageView(&viewInfo, nullptr, &ppSwapChainImageViews[i]);
		   r != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create image views!");
		}
	}
}

void VkEngineSwapChain::createRenderPass()
{
	const vk::AttachmentDescription depthAttachment(
		vk::AttachmentDescriptionFlags{},
		findDepthFormat(),
		vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eDontCare,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthStencilAttachmentOptimal);

	constexpr vk::AttachmentReference depthAttachmentRef(
		1, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	const vk::AttachmentDescription colorAttachment(vk::AttachmentDescriptionFlags{},
													mSwapChainImageFormat,
													vk::SampleCountFlagBits::e1,
													vk::AttachmentLoadOp::eClear,
													vk::AttachmentStoreOp::eStore,
													vk::AttachmentLoadOp::eDontCare,
													vk::AttachmentStoreOp::eDontCare,
													vk::ImageLayout::eUndefined,
													vk::ImageLayout::ePresentSrcKHR);

	constexpr vk::AttachmentReference colorAttachmentRef(0,
														 vk::ImageLayout::eColorAttachmentOptimal);

	const vk::SubpassDescription subpass(vk::SubpassDescriptionFlags{},
											 vk::PipelineBindPoint::eGraphics,
											 0,
											 nullptr,
											 1,
											 &colorAttachmentRef,
											 nullptr,
											 &depthAttachmentRef,
											 0,
											 nullptr);

	const vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL,
										   0,
										   vk::PipelineStageFlagBits::eColorAttachmentOutput |
											   vk::PipelineStageFlagBits::eEarlyFragmentTests,
										   vk::PipelineStageFlagBits::eColorAttachmentOutput |
											   vk::PipelineStageFlagBits::eEarlyFragmentTests,
										   vk::AccessFlagBits{},
										   vk::AccessFlagBits::eColorAttachmentWrite |
											   vk::AccessFlagBits::eDepthStencilAttachmentWrite);

	const std::array attachments = {colorAttachment, depthAttachment};

	const vk::RenderPassCreateInfo renderPassInfo(vk::RenderPassCreateFlags{},
												  static_cast<uint32_t>(attachments.size()),
												  attachments.data(),
												  1,
												  &subpass,
												  1,
												  &dependency);

	if(const vk::Result r =
		   device.device().createRenderPass(&renderPassInfo, nullptr, &pRenderPass);
	   r != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

void VkEngineSwapChain::createFramebuffers()
{
	ppSwapChainFramebuffers.resize(imageCount());

	for(size_t i = 0; i < imageCount(); i++)
	{

		std::array attachments = {ppSwapChainImageViews[i], ppDepthImageViews[i]};

		vk::FramebufferCreateInfo framebufferInfo(vk::FramebufferCreateFlags{},
												  pRenderPass,
												  static_cast<uint32_t>(attachments.size()),
												  attachments.data(),
												  getSwapChainExtent().width,
												  getSwapChainExtent().height,
												  1);

		if(const vk::Result r = device.device().createFramebuffer(
			   &framebufferInfo, nullptr, &ppSwapChainFramebuffers[i]);
		   r != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void VkEngineSwapChain::createDepthResources()
{

	depthImages.resize(imageCount());
	ppDepthImageMemorys.resize(imageCount());
	ppDepthImageViews.resize(imageCount());

	for(size_t i = 0; i < depthImages.size(); i++)
	{
		vk::ImageCreateInfo imageInfo(vk::ImageCreateFlags{},
									  vk::ImageType::e2D,
									  findDepthFormat(),
									  {getSwapChainExtent().width, getSwapChainExtent().height, 1},
									  1,
									  1,
									  vk::SampleCountFlagBits::e1,
									  vk::ImageTiling::eOptimal,
									  vk::ImageUsageFlagBits::eDepthStencilAttachment,
									  vk::SharingMode::eExclusive,
									  0,
									  nullptr,
									  vk::ImageLayout::eUndefined);

		device.createImageWithInfo(imageInfo,
								   vk::MemoryPropertyFlagBits::eDeviceLocal,
								   depthImages[i],
								   ppDepthImageMemorys[i]);

		vk::ImageViewCreateInfo viewInfo(
			vk::ImageViewCreateFlags{},
			depthImages[i],
			vk::ImageViewType::e2D,
			findDepthFormat(),
			vk::ComponentMapping{vk::ComponentSwizzle::eIdentity,
								 vk::ComponentSwizzle::eIdentity,
								 vk::ComponentSwizzle::eIdentity,
								 vk::ComponentSwizzle::eIdentity},
			vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1});

		if(const vk::Result r =
			   device.device().createImageView(&viewInfo, nullptr, &ppDepthImageViews[i]);
		   r != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create texture image view!");
		}
	}
}

void VkEngineSwapChain::createSyncObjects()
{
	ppImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	ppRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	ppInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	ppImagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

	constexpr vk::SemaphoreCreateInfo semaphoreInfo{};
	constexpr vk::FenceCreateInfo fenceInfo{};

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if(const vk::Result r = device.device().createSemaphore(
			   &semaphoreInfo, nullptr, &ppImageAvailableSemaphores[i]);
		   r != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
		if(const vk::Result r = device.device().createSemaphore(
			   &semaphoreInfo, nullptr, &ppRenderFinishedSemaphores[i]);
		   r != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
		if(const vk::Result r =
			   device.device().createFence(&fenceInfo, nullptr, &ppInFlightFences[i]);
		   r != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

vk::SurfaceFormatKHR VkEngineSwapChain::chooseSwapSurfaceFormat(
	const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
	for(const auto& availableFormat : availableFormats)
	{
		if(availableFormat.format == vk::Format::eB8G8R8A8Unorm &&
		   availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

vk::PresentModeKHR VkEngineSwapChain::chooseSwapPresentMode(
	const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
	for(const auto& availablePresentMode : availablePresentModes)
	{
		if(availablePresentMode == vk::PresentModeKHR::eFifoRelaxed)
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
	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D
VkEngineSwapChain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
{
	if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}

	vk::Extent2D actualExtent = windowExtent;
	actualExtent.width = std::max(capabilities.minImageExtent.width,
								  std::min(capabilities.maxImageExtent.width, actualExtent.width));
	actualExtent.height =
		std::max(capabilities.minImageExtent.height,
				 std::min(capabilities.maxImageExtent.height, actualExtent.height));

	return actualExtent;
}

vk::Format VkEngineSwapChain::findDepthFormat() const
{
	return device.findSupportedFormat(
		{vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

} // namespace vke