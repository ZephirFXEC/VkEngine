#include "app.hpp"

#include <chrono>
#include <core/engine_controller.hpp>

#include "engine_render_system.hpp"
#include "utils/logger.hpp"

namespace vke {

struct GlobalUBO {
	glm::mat4 view;
	glm::vec3 light = glm::normalize(glm::vec3(1.0f, -3.0f, -1.0f));
};

App::App()
    : mVkWindow(std::make_shared<VkEngineWindow>(WIDTH, HEIGHT, "VkEngine")),
      mVkDevice(mVkWindow),
      mVkRenderer(mVkDevice, *mVkWindow) {
	initImGUI();
	loadGameObjects();
}

void App::initImGUI() const {
	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(mVkWindow->getWindow(), true);

	// Init ImGUI
	ImGui_ImplVulkan_InitInfo init_info = {
	    .Instance = mVkDevice.getInstance(),
	    .PhysicalDevice = mVkDevice.getPhysicalDevice(),
	    .Device = mVkDevice.getDevice(),
	    .QueueFamily = mVkDevice.findPhysicalQueueFamilies().mGraphicsFamily.value(),
	    .Queue = mVkDevice.getGraphicsQueue(),
	    .DescriptorPool = mVkDevice.getDescriptorPool(),
	    .RenderPass = mVkRenderer.getSwapChainRenderPass(),
	    .MinImageCount = 2,
	    .ImageCount = 2,
	    .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
	};

	ImGui_ImplVulkan_Init(&init_info);
}


void App::run() {
	VkEngineBuffer globalUBO{mVkDevice,
	                         sizeof(GlobalUBO),
	                         1,
	                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	                         VMA_ALLOCATION_CREATE_MAPPED_BIT,
	                         VMA_MEMORY_USAGE_CPU_TO_GPU,
	                         mVkDevice.getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment};

	const VkEngineRenderSystem renderSystem(mVkDevice, mVkRenderer.getSwapChainRenderPass());

	VkEngineCamera camera{};
	camera.setViewTarget({-1.0f, -2.0f, -2.0f}, {0.0f, 0.0f, 2.5f});

	VkEngineGameObjects viewerObject = VkEngineGameObjects::createGameObject();
	constexpr KeyboardController cameraController{};

	auto currentTime = std::chrono::high_resolution_clock::now();
	while (!mVkWindow->shouldClose()) {
		glfwPollEvents();  // poll for events

		auto newTime = std::chrono::high_resolution_clock::now();
		const float frameTime = std::chrono::duration<float>(newTime - currentTime).count();
		currentTime = newTime;

		cameraController.moveInPlaneXZ(mVkWindow->getWindow(), frameTime, viewerObject);
		camera.setViewXYZ(viewerObject.mTransform.translation, viewerObject.mTransform.rotation);


		const float aspect = mVkRenderer.getAspectRatio();
		camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		for (size_t i = 0; i < renderSystem.getPipeline()->getPipelineData().pipelineStats.size(); ++i) {
			ImGui::Text("%s: %llu", renderSystem.getPipeline()->getPipelineData().pipelineStatNames[i].c_str(),
			            renderSystem.getPipeline()->getPipelineData().pipelineStats[i]);
		}

		ImGui::Text("%s: %f %s", "Frame Time", frameTime * 1000, "ms");


		if (auto* commandBuffer = mVkRenderer.beginFrame()) {
			const u32 frameIndex = mVkRenderer.getFrameIndex();

			GlobalUBO ubo{
			    .view = camera.getProjectionMatrix() * camera.getViewMatrix(),
			};

			// globalUBO.writeToIndex(&ubo, frameIndex);
			// VK_CHECK(globalUBO.flushIndex(frameIndex));

			// Render
			vkCmdResetQueryPool(commandBuffer, renderSystem.getPipeline()->getPipelineData().queryPool, 0, 1);
			mVkRenderer.beginSwapChainRenderPass(&commandBuffer);
			vkCmdBeginQuery(commandBuffer, renderSystem.getPipeline()->getPipelineData().queryPool, 0, 0);
			renderSystem.renderGameObjects(&commandBuffer, mVkGameObjects, camera);
			vkCmdEndQuery(commandBuffer, renderSystem.getPipeline()->getPipelineData().queryPool, 0);
			mVkRenderer.endSwapChainRenderPass(&commandBuffer);
			mVkRenderer.endFrame();
		}
		renderSystem.getPipeline()->getQueryPool();
	}

	vkDeviceWaitIdle(mVkDevice.getDevice());
}

void App::loadGameObjects() {
	VKINFO("Loading models...");


	const std::shared_ptr pVkModel =
	    VkEngineModel::createModelFromFile(mVkDevice, "C:/Users/zphrfx/Desktop/vkEngine/obj/pig.obj");

	auto game_objects = VkEngineGameObjects::createGameObject();
	game_objects.pModel = pVkModel;
	game_objects.mTransform.translation = {0.f, 0.f, 2.5f};
	game_objects.mTransform.scale = glm::vec3(-1);
	mVkGameObjects.push_back(std::move(game_objects));
}

}  // namespace vke