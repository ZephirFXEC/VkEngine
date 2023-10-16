#ifndef VKWINDOW
#define VKWINDOW

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vke {

class VkWindow {
  public:
    VkWindow(int width, int height, std::string name);
    ~VkWindow();

    bool shouldClose() { return glfwWindowShouldClose(window); }

  private:
    void initWindow();

    const int height;
    const int width;

    std::string name;
    GLFWwindow *window;
};

} // namespace vke

#endif // VKWINDOW