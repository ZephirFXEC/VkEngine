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

	static constexpr int HEIGHT = 720;
	static constexpr int WIDTH = 1280;

	void run();

   private:
	void loadGameObjects();

	VkEngineWindow mVkWindow{WIDTH, HEIGHT, "VkEngine"};  // Vulkan window

	VkEngineDevice mVkDevice{mVkWindow};

	VkEngineRenderer mVkRenderer{mVkDevice, mVkWindow};

	std::vector<VkEngineGameObjects> mVkGameObjects{};
};
}  // namespace vke
