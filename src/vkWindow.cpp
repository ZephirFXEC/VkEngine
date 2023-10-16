#include "vkWindow.hpp"

namespace vke {

VkWindow::VkWindow(int width, int height, std::string name)
    : height(height), width(width), name(name) {
    initWindow();
}

VkWindow::~VkWindow() {
    glfwDestroyWindow(window); // destroy window
    glfwTerminate();           // terminate GLFW
}

void VkWindow::initWindow() {
    glfwInit(); // initialize GLFW

    glfwWindowHint(GLFW_CLIENT_API,
                   GLFW_NO_API); // do not create an OpenGL context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // disable window resizing

    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
}
} // namespace vke