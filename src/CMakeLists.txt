cmake_minimum_required(VERSION 3.5.0)

set(PROJECT_NAME engine)


file(GLOB_RECURSE SOURCES "../src/*.cpp")
file(GLOB_RECURSE HEADERS "../src/*.hpp")

add_executable(${PROJECT_NAME}
               ${SOURCES}
               ${HEADERS})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_include_directories(${PROJECT_NAME} PUBLIC ../src)

target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan glfw glm::glm GPUOpen::VulkanMemoryAllocator fmt::fmt-header-only)
