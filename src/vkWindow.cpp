#include "vkWindow.hpp"
#include <stdexcept>

namespace vke
{

VkWindow::VkWindow(const int width, const int height, std::string name)
	: mHeight(height)
	, mWidth(width)
	, mName(std::move(name))
{
	initWindow();
}

VkWindow::~VkWindow()
{
	glfwDestroyWindow(pWindow); // destroy window
	glfwTerminate(); // terminate GLFW
}

void VkWindow::createWindowSurface(const vk::Instance instance, vk::SurfaceKHR* surface) const
{


	auto *surf = VkSurfaceKHR(*surface);

	if(glfwCreateWindowSurface(instance, pWindow, nullptr, &surf) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

	*surface = vk::SurfaceKHR(surf);
}

void VkWindow::initWindow()
{
	glfwInit(); // initialize GLFW

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // do not create an OpenGL context
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // disable window resizing

	pWindow = glfwCreateWindow(mWidth, mHeight, mName.c_str(), nullptr, nullptr);
}

bool VkWindow::shouldClose() const
{
	return glfwWindowShouldClose(pWindow) != 0;
}

VkExtent2D VkWindow::getExtent() const
{
	return {static_cast<uint32_t>(mWidth), static_cast<uint32_t>(mHeight)};
}

} // namespace vke