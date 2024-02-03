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
	loadGameObjects();
	createPipelineLayout();
	createPipeline();
}

App::~App() { vkDestroyPipelineLayout(mVkDevice.getDevice(), pVkPipelineLayout, nullptr); }

void App::run() {
	while (!mVkWindow.shouldClose()) {
		// while window is open

		if (mCurrentFrame % 1000 == 0) {
			Memory::getMemoryUsage();
		}

		glfwPollEvents();  // poll for events

		auto* commandBuffer = mVkRenderer.beginFrame();
		if (commandBuffer != VK_NULL_HANDLE) {
			mVkRenderer.beginSwapChainRenderPass(commandBuffer);
			renderGameObjects(&commandBuffer, mVkGameObjects);
			mVkRenderer.endSwapChainRenderPass(commandBuffer);
			mVkRenderer.endFrame();
		}

		++mCurrentFrame;
	}

	vkDeviceWaitIdle(mVkDevice.getDevice());
}

void App::loadGameObjects() {
	VKINFO("Loading models...");

	const std::shared_ptr pVkModel = createCubeModel(mVkDevice, mVkRenderer.getSwapChain(), {0.f, 0.f, 0.f});
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
	assert(pVkPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	VkEnginePipeline::defaultPipelineConfigInfo(pipelineConfig);

	pipelineConfig.renderPass = mVkRenderer.getRenderPass();
	pipelineConfig.pipelineLayout = pVkPipelineLayout;

	pVkPipeline =
	    std::make_unique<VkEnginePipeline>(mVkDevice, "C:/Users/zphrfx/Desktop/vkEngine/shaders/simple.vert.spv",
	                                       "C:/Users/zphrfx/Desktop/vkEngine/shaders/simple.frag.spv", pipelineConfig);
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

}  // namespace vke