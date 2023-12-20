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

    VkWindow(const VkWindow &) = delete;            // copy constructor
    VkWindow &operator=(const VkWindow &) = delete; // copy assignment operator

    bool shouldClose() const;
    void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) const;
    VkExtent2D getExtent() const;

  private:
    void initWindow(); // initialize GLFW window

    const int mHeight = 0;
    const int mWidth = 0;

    std::string mName{};   // window name
    GLFWwindow *pWindow{}; // pointer to a GLFWwindow object
};

} // namespace vke

#endif // VKWINDOW