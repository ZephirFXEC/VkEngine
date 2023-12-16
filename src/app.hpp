#ifndef APP
#define APP

#include "vkEnginePipeline.hpp"
#include "vkWindow.hpp"

namespace vke {

class App {
  public:
    static constexpr int HEIGHT = 600;
    static constexpr int WIDTH = 800;

    void run() const;

  private:
    VkWindow mVkWindow{WIDTH, HEIGHT, "VkEngine"}; // Vulkan window
    VkEngineDevice mVkDevice{mVkWindow};
    VkEnginePipeline mVkPipeline{
        mVkDevice, "../shaders/simple_shader.vert.spv",
        "../shaders/simple_shader.frag.spv",
        VkEnginePipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
};

} // namespace vke

#endif // APP