#include "app.hpp"

#include <stdexcept>

namespace vke
{
App::App()
{
	createPipelineLayout();
	createPipeline();
	createCommandBuffers();
}

App::~App()
{
	mVkDevice.device().destroyPipelineLayout(pVkPipelineLayout);
}

void App::run()
{
	while(!mVkWindow.shouldClose())
	{ // while window is open
		glfwPollEvents(); // poll for events
		//drawFrame(); // draw frame
	}

    vkDeviceWaitIdle(mVkDevice.device());
}

void App::createPipelineLayout()
{

	constexpr vk::PipelineLayoutCreateInfo pipelineLayoutInfo({}, 0, nullptr, 0, nullptr);

	if(const vk::Result result = mVkDevice.device().createPipelineLayout(
		   &pipelineLayoutInfo, nullptr, &pVkPipelineLayout);
	   result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}
}

void App::createPipeline()
{

	auto pipelineConfig =
		VkEnginePipeline::defaultPipelineConfigInfo(mVkSwapChain.width(), mVkSwapChain.height());

	pipelineConfig.renderPass = mVkSwapChain.getRenderPass();
	pipelineConfig.pipelineLayout = pVkPipelineLayout;

	pVkPipeline =
		std::make_unique<VkEnginePipeline>(mVkDevice,
										   "/Users/ecrema/Desktop/VkEngine/shaders/simple.vert.spv",
										   "/Users/ecrema/Desktop/VkEngine/shaders/simple.frag.spv",
										   pipelineConfig);
}

void App::createCommandBuffers()
{
	ppVkCommandBuffers.resize(mVkSwapChain.imageCount());

	const vk::CommandBufferAllocateInfo allocInfo(mVkDevice.getCommandPool(),
												  vk::CommandBufferLevel::ePrimary,
												  static_cast<uint32_t>(ppVkCommandBuffers.size()));

	if(const vk::Result result =
		   mVkDevice.device().allocateCommandBuffers(&allocInfo, ppVkCommandBuffers.data());
	   result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for(uint32_t i = 0; i < mVkSwapChain.imageCount(); ++i)
	{
		vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

		if(const vk::Result result = ppVkCommandBuffers[i].begin(&beginInfo);
		   result != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		std::array<vk::ClearValue, 2> clearValues{};
		clearValues[0].setColor(std::array{0.1f, 0.1f, 0.1f, 1.0f});
		clearValues[1].setDepthStencil({1.0f, 0});

		vk::RenderPassBeginInfo renderPassInfo(mVkSwapChain.getRenderPass(),
											   mVkSwapChain.getFrameBuffer(i),
											   {{0, 0}, mVkSwapChain.getSwapChainExtent()},
											   static_cast<uint32_t>(clearValues.size()),
											   clearValues.data());

		ppVkCommandBuffers[i].beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);
		pVkPipeline->bind(ppVkCommandBuffers[i]);
		ppVkCommandBuffers[i].draw(3, 1, 0, 0);
		ppVkCommandBuffers[i].endRenderPass();

		try // end recording command buffer
		{
			ppVkCommandBuffers[i].end();
		}
		catch(const std::exception& e)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

void App::drawFrame()
{
	uint32_t imageIndex = 0;

	if(const vk::Result result = mVkSwapChain.acquireNextImage(&imageIndex);
	   result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	if(const vk::Result result =
		   mVkSwapChain.submitCommandBuffers(&ppVkCommandBuffers[imageIndex], &imageIndex);
	   result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}
}

} // namespace vke