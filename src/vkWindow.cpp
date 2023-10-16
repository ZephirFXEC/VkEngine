#include "vkWindow.hpp"

namespace vke {

VkWindow::VkWindow(int width, int height, std::string name)
    : height(height), width(width), name(name) {
    initWindow();
}

VkWindow::~VkWindow() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void VkWindow::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
}
} // namespace vke