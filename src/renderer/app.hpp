#pragma once

#include "core/engine_device.hpp"
#include "core/engine_ecs.hpp"
#include "core/engine_window.hpp"
#include "engine_renderer.hpp"

namespace vke {
class App {
   public:
	App();

	~App() = default;
	App(const App&) = delete;
	App& operator=(const App&) = delete;

	static constexpr int HEIGHT = 600;
	static constexpr int WIDTH = 800;

	void run();

   private:
	void loadGameObjects();

	VkEngineWindow mVkWindow{WIDTH, HEIGHT, "VkEngine"};  // Vulkan window

	VkEngineDevice mVkDevice{mVkWindow};

	VkEngineRenderer mVkRenderer{mVkDevice, mVkWindow};

	std::vector<VkEngineGameObjects> mVkGameObjects{};


	static std::unique_ptr<VkEngineModel> createCubeModel(VkEngineDevice& device,
	                                                      std::shared_ptr<VkEngineSwapChain> swapchain,
	                                                      const glm::vec3 offset) {
		std::vector<VkEngineModel::Vertex> vertices{
		    {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},  // 0: Bottom-left-back
		    {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},   // 1: Bottom-right-back
		    {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},    // 2: Top-right-back
		    {{-.5f, .5f, -.5f}, {.1f, .8f, .1f}},   // 3: Top-left-back
		    {{-.5f, -.5f, .5f}, {.1f, .1f, .8f}},   // 4: Bottom-left-front
		    {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},    // 5: Bottom-right-front
		    {{.5f, .5f, .5f}, {.1f, .1f, .8f}},     // 6: Top-right-front
		    {{-.5f, .5f, .5f}, {.9f, .9f, .9f}}     // 7: Top-left-front
		};


		std::vector<u32> indices{// Front face
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


		VkEngineModel::MeshData meshData{.pVertices = vertices.data(),
		                                 .vCount = static_cast<u32>(vertices.size()),
		                                 .pIndices = indices.data(),
		                                 .iCount = static_cast<u32>(indices.size())};

		for (auto& [pos, col] : vertices) {
			pos += offset;
		}

		return std::make_unique<VkEngineModel>(device, swapchain, meshData);
	}
};
}  // namespace vke
