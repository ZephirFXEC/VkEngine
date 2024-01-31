#include "engine_window.hpp"

#include <GLFW/glfw3.h>

#include "utils/logger.hpp"

namespace vke {
VkEngineWindow::VkEngineWindow(const int width, const int height, std::string name)
    : mHeight(height), mWidth(width), mName(std::move(name)) {
	initWindow();
}

VkEngineWindow::~VkEngineWindow() {
	VKINFO("Destroyed window");
	glfwDestroyWindow(pWindow);  // destroy window
	glfwTerminate();             // terminate GLFW
}

void VkEngineWindow::createWindowSurface(const VkInstance* const instance, VkSurfaceKHR* surface) const {
	VK_CHECK(glfwCreateWindowSurface(*instance, pWindow, nullptr, surface));
}

void VkEngineWindow::initWindow() {
	if (glfwInit() == GLFW_FALSE) {
		throw std::runtime_error("failed to initialize GLFW!");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);  // do not create an OpenGL context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);     // disable window resizing

	pWindow = glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(pWindow, this);
	glfwSetFramebufferSizeCallback(pWindow, framebufferResizeCallback);
}

void VkEngineWindow::framebufferResizeCallback(GLFWwindow* window, const int width, const int height) {
	auto* app = static_cast<VkEngineWindow*>(glfwGetWindowUserPointer(window));
	app->mFramebufferResized = true;
	app->mWidth = width;
	app->mHeight = height;
}
}  // namespace vke