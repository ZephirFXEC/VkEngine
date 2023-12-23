#include "app.hpp"

#include <stdexcept>

namespace vke {
App::App()
{
	loadModels();
	createPipelineLayout();
	recreateSwapChain();
	createCommandBuffers();
}

App::~App()
{
	mVkDevice.getDeletionQueue().push_function([device = mVkDevice.device(), pipelineLayout = pVkPipelineLayout]() {
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	});

	mVkDevice.getDeletionQueue().flush();
}

void App::run()
{
	while (!mVkWindow.shouldClose())
	{                        // while window is open
		glfwPollEvents();    // poll for events
		drawFrame();         // draw frame
	}

	vkDeviceWaitIdle(mVkDevice.device());
}

void App::loadModels()
{
	const std::vector<VkEngineModel::Vertex> vertices = { { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
														  { { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
														  { { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
														  { { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } } };

	const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

	pVkModel = std::make_unique<VkEngineModel>(mVkDevice, vertices, indices);
}

void App::createPipelineLayout()
{
	;
	constexpr VkPipelineLayoutCreateInfo pipelineLayoutInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
															 .setLayoutCount = 0,
															 .pSetLayouts = nullptr,
															 .pushConstantRangeCount = 0,
															 .pPushConstantRanges = nullptr };

	if (vkCreatePipelineLayout(mVkDevice.device(), &pipelineLayoutInfo, nullptr, &pVkPipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

void App::createPipeline()
{
	PipelineConfigInfo pipelineConfig{};
	VkEnginePipeline::defaultPipelineConfigInfo(pipelineConfig);

	pipelineConfig.renderPass = mVkSwapChain->getRenderPass();
	pipelineConfig.pipelineLayout = pVkPipelineLayout;

	pVkPipeline = std::make_unique<VkEnginePipeline>(
		mVkDevice, "/Users/ecrema/Desktop/VkEngine/shaders/simple.vert.spv", "/Users/ecrema/Desktop/VkEngine/shaders/simple.frag.spv",
		pipelineConfig);
}

void App::createCommandBuffers()
{
	ppVkCommandBuffers.resize(mVkSwapChain->imageCount());

	const VkCommandBufferAllocateInfo allocInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
												 .commandPool = mVkDevice.getCommandPool(),
												 .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
												 .commandBufferCount = static_cast<uint32_t>(mVkSwapChain->imageCount()) };

	if (vkAllocateCommandBuffers(mVkDevice.device(), &allocInfo, ppVkCommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command buffers!");
	}
}

void App::recordCommandsBuffers(const size_t imageIndex) const
{
	constexpr VkCommandBufferBeginInfo beginInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	if (vkBeginCommandBuffer(ppVkCommandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = mVkSwapChain->getRenderPass(),
		.framebuffer = mVkSwapChain->getFrameBuffer(static_cast<uint32_t>(imageIndex)),
		.renderArea = { .offset = { 0, 0 }, .extent = mVkSwapChain->getSwapChainExtent() },
	};

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { { 0.1f, 0.1f, 0.1f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	const VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(mVkSwapChain->getSwapChainExtent().width),
		.height = static_cast<float>(mVkSwapChain->getSwapChainExtent().height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	const VkRect2D scissor{
		.offset = { 0, 0 },
		.extent = mVkSwapChain->getSwapChainExtent(),
	};

	vkCmdBeginRenderPass(ppVkCommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(ppVkCommandBuffers[imageIndex], 0, 1, &viewport);
	vkCmdSetScissor(ppVkCommandBuffers[imageIndex], 0, 1, &scissor);

	pVkPipeline->bind(ppVkCommandBuffers[imageIndex]);
	pVkModel->bind(ppVkCommandBuffers[imageIndex]);
	pVkModel->draw(ppVkCommandBuffers[imageIndex]);

	vkCmdEndRenderPass(ppVkCommandBuffers[imageIndex]);
	if (vkEndCommandBuffer(ppVkCommandBuffers[imageIndex]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}
}
void App::freeCommandBuffers()
{
	vkFreeCommandBuffers(
		mVkDevice.device(), mVkDevice.getCommandPool(), static_cast<uint32_t>(ppVkCommandBuffers.size()), ppVkCommandBuffers.data());

	ppVkCommandBuffers.clear();
}

void App::recreateSwapChain()
{
	auto extent = mVkWindow.getExtent();
	while (extent.width == 0 || extent.height == 0)
	{
		extent = mVkWindow.getExtent();
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(mVkDevice.device());

	if (mVkSwapChain == nullptr)
	{
		mVkSwapChain = std::make_unique<VkEngineSwapChain>(mVkDevice, extent);
	}
	else
	{
		mVkSwapChain = std::make_unique<VkEngineSwapChain>(mVkDevice, extent, std::move(mVkSwapChain));

		if (mVkSwapChain->imageCount() != ppVkCommandBuffers.size())
		{
			freeCommandBuffers();
			createCommandBuffers();
		}
	}

	createPipeline();
}

void App::drawFrame()
{
	uint32_t imageIndex = 0;
	auto     result = mVkSwapChain->acquireNextImage(&imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	recordCommandsBuffers(imageIndex);
	result = mVkSwapChain->submitCommandBuffers(&ppVkCommandBuffers[imageIndex], &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mVkWindow.wasWindowResized())
	{
		mVkWindow.resetWindowResizedFlag();
		recreateSwapChain();
		return;
	}

	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}
}

}    // namespace vke