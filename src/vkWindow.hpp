#ifndef VKWINDOW
#define VKWINDOW

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vke {

class VkWindow {
  public:
    VkWindow(int height, int width, std::string name);
    ~VkWindow();

  private:
    void initWindow();

    const int height;
    const int width;

    std::string name;
    GLFWwindow *window;
};

} // namespace vke

#endif // VKWINDOW