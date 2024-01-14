#pragma once

#include "engine_device.hpp"
#include "engine_model.hpp"
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
	void loadModels();

	void createPipelineLayout();

	void createPipeline();

	void createCommandBuffers();

	void drawFrame();

	void recreateSwapChain();

	void recordCommandsBuffers(size_t imageIndex) const;

	void freeCommandBuffers() const;

	VkEngineWindow mVkWindow{WIDTH, HEIGHT, "VkEngine"};  // Vulkan window

	VkEngineDevice mVkDevice{mVkWindow};

	std::shared_ptr<VkEngineSwapChain> mVkSwapChain = nullptr;
	std::unique_ptr<VkEnginePipeline> pVkPipeline = nullptr;
	std::unique_ptr<VkEngineModel> pVkModel = nullptr;

	VkPipelineLayout pVkPipelineLayout = VK_NULL_HANDLE;

	struct CommandBuffer {
		VkCommandBuffer* ppVkCommandBuffers{};
		u8 mSize{};  // number of command buffers (could be uint8_t)
	} mCommandBuffer{};

	static inline u64 mCurrentFrame = 0;
};
}  // namespace vke