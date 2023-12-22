#ifndef APP_HPP
#define APP_HPP

#include "vkEngineDevice.hpp"
#include "vkEnginePipeline.hpp"
#include "vkEngineSwapChain.hpp"
#include "vkEngineWindow.hpp"
#include "vkEngineModel.hpp"

#include <memory>

namespace vke
{

class App
{
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

	VkEngineWindow mVkWindow{WIDTH, HEIGHT, "VkEngine"}; // Vulkan window

	VkEngineDevice mVkDevice{mVkWindow};

	std::unique_ptr<VkEngineSwapChain> mVkSwapChain = nullptr;
	std::unique_ptr <VkEnginePipeline> pVkPipeline = nullptr;
	std::unique_ptr<VkEngineModel> pVkModel = nullptr;

	VkPipelineLayout pVkPipelineLayout = VK_NULL_HANDLE;
	std::vector <VkCommandBuffer> ppVkCommandBuffers{};
};

} // namespace vke

#endif // APP