#include "app.hpp"

#include "engine_render_system.hpp"
#include "utils/logger.hpp"
#include "utils/memory.hpp"
#include "utils/types.hpp"

namespace vke {
App::App() { loadGameObjects(); }

void App::run() {
	const VkEngineRenderSystem renderSystem(mVkDevice, &mVkRenderer.getSwapChain()->getRenderPass());

	u64 frame = 0;
	while (!mVkWindow.shouldClose()) {
		// while window is open

		if (frame % 1000 == 0) {
			Memory::getMemoryUsage();
		}

		glfwPollEvents();  // poll for events

		auto* commandBuffer = mVkRenderer.beginFrame();
		if (commandBuffer != VK_NULL_HANDLE) {
			mVkRenderer.beginSwapChainRenderPass(&commandBuffer);
			renderSystem.renderGameObjects(&commandBuffer, mVkGameObjects);
			mVkRenderer.endSwapChainRenderPass(&commandBuffer);
			mVkRenderer.endFrame();
		}

		++frame;
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

}  // namespace vke