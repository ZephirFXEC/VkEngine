#include "vkWindow.hpp"
#include <stdexcept>

namespace vke {

VkWindow::VkWindow(const int width, const int height, std::string name)
    : mHeight(height), mWidth(width), mName(std::move(name)) {
    initWindow();
}

VkWindow::~VkWindow() {
    glfwDestroyWindow(pWindow); // destroy window
    glfwTerminate();            // terminate GLFW
}

void VkWindow::createWindowSurface(VkInstance instance,
                                   VkSurfaceKHR *surface) const {
    if (glfwCreateWindowSurface(instance, pWindow, nullptr, surface) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void VkWindow::initWindow() {
    glfwInit(); // initialize GLFW

    glfwWindowHint(GLFW_CLIENT_API,
                   GLFW_NO_API); // do not create an OpenGL context
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // disable window resizing

    pWindow =
        glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr);
}
} // namespace vke