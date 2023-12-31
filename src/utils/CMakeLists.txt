cmake_minimum_required(VERSION 3.5.0)

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.hpp")

add_library(utils STATIC ${SOURCES} ${HEADERS})

set_property(TARGET utils PROPERTY CXX_STANDARD 20)
target_compile_features(utils PUBLIC cxx_std_20)

target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../core)

target_link_libraries(utils GPUOpen::VulkanMemoryAllocator fmt::fmt)