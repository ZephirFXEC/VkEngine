#include "vkEngineWindow.hpp"
#include <stdexcept>

namespace vke
{

VkEngineWindow::VkEngineWindow(const int width, const int height, std::string name)
	: mHeight(height), mWidth(width), mName(std::move(name))
{
	initWindow();
}

VkEngineWindow::~VkEngineWindow()
{
	glfwDestroyWindow(pWindow); // destroy window
	glfwTerminate(); // terminate GLFW
}

void VkEngineWindow::createWindowSurface(const VkInstance instance, VkSurfaceKHR* surface) const
{

	if(glfwCreateWindowSurface(instance, pWindow, nullptr, surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

void VkEngineWindow::initWindow()
{
	glfwInit(); // initialize GLFW

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // do not create an OpenGL context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); // disable window resizing

	pWindow = glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(pWindow, this);
	glfwSetFramebufferSizeCallback(pWindow, framebufferResizeCallback);
}

void VkEngineWindow::framebufferResizeCallback(GLFWwindow* window, const int width, const int height)
{
	auto *app = static_cast<VkEngineWindow*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
	app->mWidth = width;
	app->mHeight = height;
}


bool VkEngineWindow::shouldClose() const
{
	return glfwWindowShouldClose(pWindow) != 0;
}

VkExtent2D VkEngineWindow::getExtent() const
{
	return {static_cast <uint32_t>(mWidth), static_cast <uint32_t>(mHeight)};
}

} // namespace vke