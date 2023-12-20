#ifndef APP
#define APP

#include "vkEngineDevice.hpp"
#include "vkEnginePipeline.hpp"
#include "vkEngineSwapChain.hpp"
#include "vkWindow.hpp"

#include <memory>

namespace vke {

class App {
  public:
    App();
    ~App();

    App(const App &) = delete;
    App &operator=(const App &) = delete;

    static constexpr int HEIGHT = 600;
    static constexpr int WIDTH = 800;

    void run();

  private:
    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffers();
    void drawFrame();

    VkWindow mVkWindow{WIDTH, HEIGHT, "VkEngine"}; // Vulkan window

    VkEngineDevice mVkDevice{mVkWindow};

    VkEngineSwapChain mVkSwapChain{mVkDevice, mVkWindow.getExtent()};

    std::unique_ptr<VkEnginePipeline> pVkPipeline = nullptr;

    VkPipelineLayout pVkPipelineLayout = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> ppVkCommandBuffers{};
};

} // namespace vke

#endif // APP