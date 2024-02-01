#pragma once

#include "engine_device.hpp"
#include "engine_ecs.hpp"
#include "engine_pipeline.hpp"
#include "engine_swapchain.hpp"
#include "engine_window.hpp"

namespace vke {
class App {
   public:
	App();

	~App();

	App(const App&) = delete;

	App& operator=(const App&) = delete;

	static constexpr int HEIGHT = 600;
	static constexpr int WIDTH = 800;

	void run();

   private:
	void loadGameObjects();

	void createPipelineLayout();

	void createPipeline();

	void createCommandBuffers();

	void drawFrame();

	void recreateSwapChain();

	void recordCommandsBuffers(size_t imageIndex);

	void renderGameObjects(const VkCommandBuffer* commandBuffer, std::vector<VkEngineGameObjects>& objects) const;

	void freeCommandBuffers() const;

	VkEngineWindow mVkWindow{WIDTH, HEIGHT, "VkEngine"};  // Vulkan window

	VkEngineDevice mVkDevice{mVkWindow};

	std::shared_ptr<VkEngineSwapChain> mVkSwapChain = nullptr;
	std::unique_ptr<VkEnginePipeline> pVkPipeline = nullptr;
	std::vector<VkEngineGameObjects> mVkGameObjects{};

	VkPipelineLayout pVkPipelineLayout = VK_NULL_HANDLE;

	struct CommandBuffer {
		VkCommandBuffer* ppVkCommandBuffers{};
		u32 mSize{};  // number of command buffers (could be uint8_t)
	} mCommandBuffer{};

	static inline u64 mCurrentFrame = 0;

	static std::unique_ptr<VkEngineModel> createCubeModel(VkEngineDevice& device, std::shared_ptr<VkEngineSwapChain> swapchain, const glm::vec3 offset) {

		std::vector<VkEngineModel::Vertex> vertices = {
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},  // 0: Bottom-left-back
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},   // 1: Bottom-right-back
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},    // 2: Top-right-back
			{{-.5f, .5f, -.5f}, {.1f, .8f, .1f}},   // 3: Top-left-back
			{{-.5f, -.5f, .5f}, {.1f, .1f, .8f}},   // 4: Bottom-left-front
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},    // 5: Bottom-right-front
			{{.5f, .5f, .5f}, {.1f, .1f, .8f}},     // 6: Top-right-front
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}}     // 7: Top-left-front
		};


		const std::vector<u32> indices = {
			// Front face
			4, 5, 6, 6, 7, 4,
			// Back face
			0, 3, 2, 2, 1, 0,
			// Left face
			4, 7, 3, 3, 0, 4,
			// Right face
			5, 1, 2, 2, 6, 5,
			// Top face
			7, 6, 2, 2, 3, 7,
			// Bottom face
			4, 0, 1, 1, 5, 4};


		for (auto& [position, color] : vertices) {
			position += offset;
		}
		return std::make_unique<VkEngineModel>(device, swapchain,
			vertices.data(), vertices.size(),
			indices.data(), indices.size());
	}
};
}  // namespace vke
