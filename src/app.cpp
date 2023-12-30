#include "app.hpp"

namespace vke {
App::App() {
	createPipelineLayout();
	recreateSwapChain();
	loadModels();
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

	pVkModel =
	    std::make_unique<VkEngineModel>(mVkDevice, mVkSwapChain, vertices.data(), vCount, indices.data(), iCount);
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
	mCommandBuffer.mSize = mVkSwapChain->getImageCount();
	mCommandBuffer.ppVkCommandBuffers = new VkCommandBuffer[mCommandBuffer.mSize];

	const VkCommandBufferAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	                                            .commandPool = mVkSwapChain->getCommandPool(),
	                                            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	                                            .commandBufferCount = mCommandBuffer.mSize};

	VK_CHECK(vkAllocateCommandBuffers(mVkDevice.getDevice(), &allocInfo, mCommandBuffer.ppVkCommandBuffers));
}

void App::recordCommandsBuffers(const size_t imageIndex) const {
	constexpr VkCommandBufferBeginInfo beginInfo{
	    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	VK_CHECK(vkBeginCommandBuffer(mCommandBuffer.ppVkCommandBuffers[imageIndex], &beginInfo));

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
	clearValues[1].depthStencil = {1.0f, 0};

	const VkRenderPassBeginInfo renderPassInfo{
	    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
	    .renderPass = mVkSwapChain->getRenderPass(),
	    .framebuffer = mVkSwapChain->getFrameBuffer(static_cast<uint32_t>(imageIndex)),
	    .renderArea = {.offset = {0, 0}, .extent = mVkSwapChain->getSwapChainExtent()},
	    .clearValueCount = static_cast<uint32_t>(clearValues.size()),
	    .pClearValues = clearValues.data(),
	};

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

	vkCmdBeginRenderPass(mCommandBuffer.ppVkCommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(mCommandBuffer.ppVkCommandBuffers[imageIndex], 0, 1, &viewport);
	vkCmdSetScissor(mCommandBuffer.ppVkCommandBuffers[imageIndex], 0, 1, &scissor);

	pVkPipeline->bind(&mCommandBuffer.ppVkCommandBuffers[imageIndex]);
	pVkModel->bind(&mCommandBuffer.ppVkCommandBuffers[imageIndex]);
	pVkModel->draw(&mCommandBuffer.ppVkCommandBuffers[imageIndex]);

	vkCmdEndRenderPass(mCommandBuffer.ppVkCommandBuffers[imageIndex]);

	VK_CHECK(vkEndCommandBuffer(mCommandBuffer.ppVkCommandBuffers[imageIndex]));
}

void App::freeCommandBuffers() const {
	vkFreeCommandBuffers(mVkDevice.getDevice(), mVkSwapChain->getCommandPool(), mCommandBuffer.mSize,
	                     mCommandBuffer.ppVkCommandBuffers);
	delete[] mCommandBuffer.ppVkCommandBuffers;
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

		if (mVkSwapChain->getImageCount() != mCommandBuffer.mSize) {
			freeCommandBuffers();
			createCommandBuffers();
		}
	}

	createPipeline();
}

void App::drawFrame() {
	uint32_t imageIndex = 0;
	VkResult result = mVkSwapChain->acquireNextImage(&imageIndex);

	// Combining the check for VK_ERROR_OUT_OF_DATE_KHR with VK_SUBOPTIMAL_KHR.
	// Also including the window resize check here to avoid duplicate code.
	if (result == VK_ERROR_OUT_OF_DATE_KHR || mVkWindow.wasWindowResized() || result == VK_SUBOPTIMAL_KHR) {
		mVkWindow.resetWindowResizedFlag();
		recreateSwapChain();
		return;  // Early return to avoid further processing since swapchain needs recreation.
	}

	// Consolidating error handling for acquisition failure into one check.
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	// Moved recordCommandsBuffers inside the success conditional to avoid calling it
	// when swapchain recreation is needed or an error occurs.
	recordCommandsBuffers(imageIndex);

	result = mVkSwapChain->submitCommandBuffers(&mCommandBuffer.ppVkCommandBuffers[imageIndex], &imageIndex);

	// Checking for swap chain-related errors after submitting command buffers.
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		recreateSwapChain();
		return;
	}

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swap chain image!");
	}
}
}  // namespace vke