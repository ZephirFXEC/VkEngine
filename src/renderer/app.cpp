#include "app.hpp"


#include <chrono>
#include <core/engine_controller.hpp>

#include "engine_render_system.hpp"

#include <imgui_impl_glfw.h>
#include "imgui_impl_vulkan.h"

#include "utils/logger.hpp"

namespace vke {
App::App() { loadGameObjects(); }

void App::run() {
	const VkEngineRenderSystem renderSystem(mVkDevice, mVkRenderer.getSwapChainRenderPass());
	VkEngineCamera camera{};
	camera.setViewTarget({-1.0f, -2.0f, -2.0f},{0.0f, 0.0f, 2.5f});

	auto viewerObject = VkEngineGameObjects::createGameObject();
	constexpr KeyboardController cameraController{};

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(mVkWindow.getWindow(), true);

	ImGui_ImplVulkan_InitInfo init_info = {
		.Instance = mVkDevice.getInstance(),
		.PhysicalDevice = mVkDevice.getPhysicalDevice(),
		.Device = mVkDevice.getDevice(),
		.QueueFamily = mVkDevice.findPhysicalQueueFamilies().mGraphicsFamily.value(),
		.Queue = mVkDevice.getGraphicsQueue(),
		.DescriptorPool = mVkDevice.getDescriptorPool(),
		.PipelineCache = VK_NULL_HANDLE,
		.RenderPass = mVkRenderer.getSwapChainRenderPass(),
		.MinImageCount = 2,
		.Subpass = 0,
		.ImageCount = 2,
		.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
		.Allocator = nullptr,
		.CheckVkResultFn = nullptr,
	};

	ImGui_ImplVulkan_Init(&init_info);


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

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();

		ImGui::ShowDemoWindow();


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