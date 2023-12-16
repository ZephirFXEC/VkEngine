#include "app.hpp"

namespace vke {
void App::run() const {
    while (!mVkWindow.shouldClose()) { // while window is open
        glfwPollEvents();              // poll for events
    }
}
} // namespace vke