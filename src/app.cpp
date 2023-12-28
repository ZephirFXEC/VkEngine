#include "app.hpp"

#include <stdexcept>

namespace vke {
App::App() {
	loadModels();
	createPipelineLayout();
	recreateSwapChain();
	createCommandBuffers();
}

App::~App() { vkDestroyPipelineLayout(mVkDevice.getDevice(), pVkPipelineLayout, nullptr); }

void App::run() {
	while (!mVkWindow.shouldClose()) {
		// while window is open
		glfwPollEvents();  // poll for events
		drawFrame();       // draw frame
	}

	vkDeviceWaitIdle(mVkDevice.getDevice());
}

void App::loadModels() {
	constexpr uint32_t iCount = 6;
	constexpr uint32_t vCount = 4;

	constexpr std::array<VkEngineModel::Vertex, vCount> vertices{
	    VkEngineModel::Vertex{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},  // 0
	    VkEngineModel::Vertex{{0.5f, -0.5}, {0.0f, 1.0f, 0.0f}},    // 1
	    VkEngineModel::Vertex{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},    // 2
	    VkEngineModel::Vertex{{-0.5f, 0.5f}, {1.0f, 0.0f, 1.0f}},   // 3
	};

	constexpr std::array<uint32_t, iCount> indices{0, 1, 2, 2, 3, 0};

	pVkModel = std::make_unique<VkEngineModel>(mVkDevice, vertices.data(), vCount, indices.data(), iCount);
}

void App::createPipelineLayout() {
	constexpr VkPipelineLayoutCreateInfo pipelineLayoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	                                                        .setLayoutCount = 0,
	                                                        .pSetLayouts = nullptr,
	                                                        .pushConstantRangeCount = 0,
	                                                        .pPushConstantRanges = nullptr};

	VK_CHECK(vkCreatePipelineLayout(mVkDevice.getDevice(), &pipelineLayoutInfo, nullptr, &pVkPipelineLayout));
}

void App::createPipeline() {
	assert(mVkSwapChain != nullptr && "Cannot create pipeline before swap chain");
	assert(pVkPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	VkEnginePipeline::defaultPipelineConfigInfo(pipelineConfig);

	pipelineConfig.renderPass = mVkSwapChain->getRenderPass();
	pipelineConfig.pipelineLayout = pVkPipelineLayout;

	pVkPipeline =
	    std::make_unique<VkEnginePipeline>(mVkDevice, "/Users/ecrema/Desktop/VkEngine/shaders/simple.vert.spv",
	                                       "/Users/ecrema/Desktop/VkEngine/shaders/simple.frag.spv", pipelineConfig);
}

void App::createCommandBuffers() {
	ppVkCommandBuffers.resize(mVkSwapChain->imageCount());

	const VkCommandBufferAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	                                            .commandPool = mVkDevice.getCommandPool(),
	                                            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	                                            .commandBufferCount = static_cast<uint32_t>(ppVkCommandBuffers.size())};

	VK_CHECK(vkAllocateCommandBuffers(mVkDevice.getDevice(), &allocInfo, ppVkCommandBuffers.data()));
}

void App::recordCommandsBuffers(const size_t imageIndex) const {
	constexpr VkCommandBufferBeginInfo beginInfo{
	    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	VK_CHECK(vkBeginCommandBuffer(ppVkCommandBuffers[imageIndex], &beginInfo));

	VkRenderPassBeginInfo renderPassInfo{
	    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
	    .renderPass = mVkSwapChain->getRenderPass(),
	    .framebuffer = mVkSwapChain->getFrameBuffer(static_cast<uint32_t>(imageIndex)),
	    .renderArea = {.offset = {0, 0}, .extent = mVkSwapChain->getSwapChainExtent()},
	};

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
	clearValues[1].depthStencil = {1.0f, 0};

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
	    .offset = {0, 0},
	    .extent = mVkSwapChain->getSwapChainExtent(),
	};

	vkCmdBeginRenderPass(ppVkCommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(ppVkCommandBuffers[imageIndex], 0, 1, &viewport);
	vkCmdSetScissor(ppVkCommandBuffers[imageIndex], 0, 1, &scissor);

	pVkPipeline->bind(&ppVkCommandBuffers[imageIndex]);
	pVkModel->bind(&ppVkCommandBuffers[imageIndex]);
	pVkModel->draw(&ppVkCommandBuffers[imageIndex]);

	vkCmdEndRenderPass(ppVkCommandBuffers[imageIndex]);

	VK_CHECK(vkEndCommandBuffer(ppVkCommandBuffers[imageIndex]));
}

void App::freeCommandBuffers() const {
	vkFreeCommandBuffers(mVkDevice.getDevice(), mVkDevice.getCommandPool(),
	                     static_cast<uint32_t>(ppVkCommandBuffers.size()), ppVkCommandBuffers.data());
}

void App::recreateSwapChain() {
	auto extent = mVkWindow.getExtent();

	while (extent.width == 0 || extent.height == 0) {
		extent = mVkWindow.getExtent();
		glfwWaitEvents();
	}

	VK_CHECK(vkDeviceWaitIdle(mVkDevice.getDevice()));

	if (mVkSwapChain == nullptr) {
		mVkSwapChain = std::make_unique<VkEngineSwapChain>(mVkDevice, extent);
	} else {
		mVkSwapChain = std::make_unique<VkEngineSwapChain>(mVkDevice, extent, std::move(mVkSwapChain));

		if (mVkSwapChain->imageCount() != ppVkCommandBuffers.size()) {
			freeCommandBuffers();
			createCommandBuffers();
		}
	}

	createPipeline();
}

void App::drawFrame() {
	uint32_t imageIndex = 0;
	auto result = mVkSwapChain->acquireNextImage(&imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
	}

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	recordCommandsBuffers(imageIndex);
	result = mVkSwapChain->submitCommandBuffers(&ppVkCommandBuffers[imageIndex], &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || mVkWindow.wasWindowResized()) {
		mVkWindow.resetWindowResizedFlag();
		recreateSwapChain();
		return;
	}

	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}
}
}  // namespace vke