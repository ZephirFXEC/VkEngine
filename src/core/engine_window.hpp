#pragma once


#include <string>

#include "utils/types.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Volk headers
#ifdef IMGUI_IMPL_VULKAN_USE_VOLK
#define VOLK_IMPLEMENTATION
#include <Volk/volk.h>
#endif

namespace vke {
class VkEngineWindow {
   public:
	explicit VkEngineWindow() = default;                      // default constructor
	VkEngineWindow(int width, int height, std::string name);  // constructor
	~VkEngineWindow();                                        // destructor

	VkEngineWindow(const VkEngineWindow&) = delete;             // copy constructor
	VkEngineWindow& operator=(const VkEngineWindow&) = delete;  // copy assignment operator

	[[nodiscard]] bool shouldClose() const { return glfwWindowShouldClose(pWindow) != 0; }

	[[nodiscard]] VkExtent2D getExtent() const { return {static_cast<u32>(mWidth), static_cast<u32>(mHeight)}; }

	[[nodiscard]] bool wasWindowResized() const { return mFramebufferResized; }

	void createWindowSurface(const VkInstance* instance, VkSurfaceKHR* surface) const;

	void resetWindowResizedFlag() { mFramebufferResized = false; }

	GLFWwindow* getWindow() const { return pWindow; }

   private:
	void initWindow();  // initialize GLFW window
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	int mHeight = 0;
	int mWidth = 0;
	bool mFramebufferResized = false;

	std::string mName{};    // window name
	GLFWwindow* pWindow{};  // pointer to a GLFWwindow object
};
}  // namespace vke