cmake_minimum_required(VERSION 3.5.0)

set(PROJECT_NAME utils)

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.hpp")


add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../core)


target_link_libraries(${PROJECT_NAME} GPUOpen::VulkanMemoryAllocator fmt::fmt)