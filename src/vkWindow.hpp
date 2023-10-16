#ifndef VKWINDOW
#define VKWINDOW

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vke {

class VkWindow {
  public:
    VkWindow(int width, int height, std::string name); // constructor
    ~VkWindow();                                       // destructor

    bool shouldClose() { return glfwWindowShouldClose(window); }

  private:
    void initWindow(); // initialize GLFW window

    const int height;
    const int width;

    std::string name;   // window name
    GLFWwindow *window; // pointer to a GLFWwindow object
};

} // namespace vke

#endif // VKWINDOW