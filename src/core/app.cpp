#include "app.hpp"

#include "utils/logger.hpp"
#include "utils/memory.hpp"
#include "utils/types.hpp"

#include <glm/glm.hpp>

namespace vke {
struct PushConstants {
	glm::mat4 transform{1.f};
	alignas(16) glm::vec3 color{};
};

App::App() {
	createPipelineLayout();
	recreateSwapChain();
	loadGameObjects();
	createCommandBuffers();
}

App::~App() { vkDestroyPipelineLayout(mVkDevice.getDevice(), pVkPipelineLayout, nullptr); }

void App::run() {
	while (!mVkWindow.shouldClose()) {
		// while window is open

		if (mCurrentFrame % 1000 == 0) {
			Memory::getMemoryUsage();
		}

		glfwPollEvents();  // poll for events

		drawFrame();  // draw frame

		++mCurrentFrame;
	}

	vkDeviceWaitIdle(mVkDevice.getDevice());
}

void App::loadGameObjects() {
	VKINFO("Loading models...");

	const std::shared_ptr pVkModel = createCubeModel(mVkDevice, mVkSwapChain, {0.f, 0.f, 0.f});
	auto cube = VkEngineGameObjects::createGameObject();
	cube.pModel = pVkModel;
	cube.mTransform.translation = {0.f, 0.f, 0.5f};
	cube.mTransform.scale = {0.5f, 0.5f, 0.5f};

	mVkGameObjects.push_back(std::move(cube));
}

void App::createPipelineLayout() {
	static constexpr VkPushConstantRange pushConstantRange{
	    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
	    .offset = 0,
	    .size = sizeof(PushConstants),
	};

	constexpr VkPipelineLayoutCreateInfo pipelineLayoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	                                                        .pNext = nullptr,
	                                                        .setLayoutCount = 0,
	                                                        .pSetLayouts = nullptr,
	                                                        .pushConstantRangeCount = 1,
	                                                        .pPushConstantRanges = &pushConstantRange};

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
	    std::make_unique<VkEnginePipeline>(mVkDevice, "C:/Users/zphrfx/Desktop/vkEngine/shaders/simple.vert.spv",
	                                       "C:/Users/zphrfx/Desktop/vkEngine/shaders/simple.frag.spv", pipelineConfig);
}

void App::createCommandBuffers() {
	mCommandBuffer = {
	    .ppVkCommandBuffers = Memory::allocMemory<VkCommandBuffer>(mVkSwapChain->getImageCount(), MEMORY_TAG_VULKAN),
	    .mSize = mVkSwapChain->getImageCount()};

	const VkCommandBufferAllocateInfo allocInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	                                            .commandPool = mVkSwapChain->getCommandPool(),
	                                            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	                                            .commandBufferCount = mCommandBuffer.mSize};

	VK_CHECK(vkAllocateCommandBuffers(mVkDevice.getDevice(), &allocInfo, mCommandBuffer.ppVkCommandBuffers));
}

void App::recordCommandsBuffers(const size_t imageIndex) {
	constexpr VkCommandBufferBeginInfo beginInfo{
	    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	VK_CHECK(vkBeginCommandBuffer(mCommandBuffer.ppVkCommandBuffers[imageIndex], &beginInfo));

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {{0.0f, 0.0f, 0.0f, 0.0f}};
	clearValues[1].depthStencil = {1.0f, 0};

	const VkRenderPassBeginInfo renderPassInfo{
	    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
	    .renderPass = mVkSwapChain->getRenderPass(),
	    .framebuffer = mVkSwapChain->getFrameBuffer(static_cast<u32>(imageIndex)),
	    .renderArea = {.offset = {0, 0}, .extent = mVkSwapChain->getSwapChainExtent()},
	    .clearValueCount = static_cast<u32>(clearValues.size()),
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

	renderGameObjects(&mCommandBuffer.ppVkCommandBuffers[imageIndex], mVkGameObjects);

	vkCmdEndRenderPass(mCommandBuffer.ppVkCommandBuffers[imageIndex]);

	VK_CHECK(vkEndCommandBuffer(mCommandBuffer.ppVkCommandBuffers[imageIndex]));
}

void App::freeCommandBuffers() const {

	vkResetCommandPool(mVkDevice.getDevice(), mVkSwapChain->getCommandPool(), 0);

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

void App::renderGameObjects(const VkCommandBuffer* const commandBuffer,
                            std::vector<VkEngineGameObjects>& objects) const {
	pVkPipeline->bind(commandBuffer);

	for (auto& gameObject : objects) {

		gameObject.mTransform.rotation.y += 0.01f;
		gameObject.mTransform.rotation.x += 0.005f;

		const PushConstants pushConstants{
		    .transform = gameObject.mTransform.mat4(),
		    .color = gameObject.mColor,
		};

		vkCmdPushConstants(*commandBuffer, pVkPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		                   0, sizeof(PushConstants), &pushConstants);

		gameObject.pModel->bind(commandBuffer);
		gameObject.pModel->draw(commandBuffer);
	}
}


void App::drawFrame() {
	u32 imageIndex = 0;
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