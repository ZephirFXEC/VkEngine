#include "app.hpp"

namespace vke {
void App::run() {
    while (!vkWindow.shouldClose()) { // while window is open
        glfwPollEvents();             // poll for events
    }
}
} // namespace vke