#include "app.hpp"

namespace vke {
void App::run() {
    while (!vkWindow.shouldClose()) {
        glfwPollEvents();
    }
}
} // namespace vke