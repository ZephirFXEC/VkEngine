cmake_minimum_required(VERSION 3.5.0)

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.hpp")


add_executable(engine
               ${SOURCES}
               ${HEADERS})

set_property(TARGET engine PROPERTY CXX_STANDARD 20)
target_compile_features(engine PUBLIC cxx_std_20)
target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(engine PUBLIC Vulkan::Vulkan glfw glm::glm GPUOpen::VulkanMemoryAllocator fmt::fmt)

target_precompile_headers(engine PUBLIC
                          <unordered_set>
                          <vector>
                          <memory>
                          <string>
                          <vector>
                          <functional>
                          <unordered_map>
                          <vulkan/vulkan.hpp>
                          <vulkan/vulkan.h>
                          <GLFW/glfw3.h>
                          <glm/glm.hpp>
                          <cassert>
                          <fstream>
                          <iostream>
                          <stdexcept>)