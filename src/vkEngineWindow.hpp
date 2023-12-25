#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace vke {
class VkEngineWindow {
   public:
	explicit VkEngineWindow() = default;                      // default constructor
	VkEngineWindow(int width, int height, std::string name);  // constructor
	~VkEngineWindow();                                        // destructor

	VkEngineWindow(const VkEngineWindow&) = delete;             // copy constructor
	VkEngineWindow& operator=(const VkEngineWindow&) = delete;  // copy assignment operator

	[[nodiscard]] bool shouldClose() const;

	[[nodiscard]] VkExtent2D getExtent() const;

	[[nodiscard]] bool wasWindowResized() const { return framebufferResized; }

	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) const;

	void resetWindowResizedFlag() { framebufferResized = false; }

   private:
	void initWindow();  // initialize GLFW window
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	int mHeight = 0;
	int mWidth = 0;
	bool framebufferResized = false;

	std::string mName{};    // window name
	GLFWwindow* pWindow{};  // pointer to a GLFWwindow object
};
}  // namespace vke