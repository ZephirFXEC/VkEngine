#ifndef APP
#define APP

#include "vkWindow.hpp"

namespace vke {

class App {
  public:
    static constexpr int HEIGHT = 600;
    static constexpr int WIDTH = 800;

    void run();

  private:
    VkWindow vkWindow{WIDTH, HEIGHT, "VkEngine"};
};

} // namespace vke

#endif // APP