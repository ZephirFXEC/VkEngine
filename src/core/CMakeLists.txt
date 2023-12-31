cmake_minimum_required(VERSION 3.5.0)

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.hpp")

set(PROJECT_NAME engine)

add_executable(${PROJECT_NAME}
               ${SOURCES}
               ${HEADERS})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../utils)

target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan glfw glm::glm)
target_link_libraries(${PROJECT_NAME} PUBLIC utils)