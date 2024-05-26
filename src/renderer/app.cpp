#include "app.hpp"

#include <chrono>
#include <core/engine_controller.hpp>

#include "engine_render_system.hpp"
#include "utils/logger.hpp"

namespace vke {
App::App() { loadGameObjects(); }

void App::run() {
	const VkEngineRenderSystem renderSystem(mVkDevice, mVkRenderer.getSwapChainRenderPass());
	VkEngineCamera camera{};
	camera.setViewTarget({-1.0f, -2.0f, -2.0f},{0.0f, 0.0f, 2.5f});

	auto viewerObject = VkEngineGameObjects::createGameObject();
	constexpr KeyboardController cameraController{};

	auto currentTime = std::chrono::high_resolution_clock::now();
	while (!mVkWindow.shouldClose()) {
		glfwPollEvents();  // poll for events

		auto newTime = std::chrono::high_resolution_clock::now();
		const float frameTime = std::chrono::duration<float>(newTime - currentTime).count();
		currentTime = newTime;

		cameraController.moveInPlaneXZ(mVkWindow.getWindow(), frameTime, viewerObject);
		camera.setViewXYZ(viewerObject.mTransform.translation, viewerObject.mTransform.rotation);


		const float aspect = mVkRenderer.getAspectRatio();
		camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

		if (auto* commandBuffer = mVkRenderer.beginFrame()) {
			mVkRenderer.beginSwapChainRenderPass(&commandBuffer);
			renderSystem.renderGameObjects(&commandBuffer, mVkGameObjects, camera);
			mVkRenderer.endSwapChainRenderPass(&commandBuffer);
			mVkRenderer.endFrame();
		}

	}

	vkDeviceWaitIdle(mVkDevice.getDevice());
}

void App::loadGameObjects() {
	VKINFO("Loading models...");


	const std::shared_ptr pVkModel = VkEngineModel::createModelFromFile(mVkDevice, "C:/Users/zphrfx/Desktop/vkEngine/obj/pig.obj");

	auto game_objects = VkEngineGameObjects::createGameObject();
	game_objects.pModel = pVkModel;
	game_objects.mTransform.translation = {0.f, 0.f, 2.5f};
	game_objects.mTransform.scale = glm::vec3(-1);

	mVkGameObjects.push_back(std::move(game_objects));
}

}  // namespace vke