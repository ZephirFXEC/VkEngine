file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.hpp")

add_executable(engine
        ${SOURCES}
        ${HEADERS}
)

set_property(TARGET engine PROPERTY CXX_STANDARD 20)
target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(engine PUBLIC Vulkan::Vulkan glfw)
target_precompile_headers(engine PUBLIC <optional> <vector> <memory> <string> <vector> <unordered_map> <vulkan/vulkan.h>)