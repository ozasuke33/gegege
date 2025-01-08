cmake_minimum_required(VERSION 3.7)

project(gegege_proj LANGUAGES C CXX)

find_package(Vulkan REQUIRED)

add_subdirectory(thirdparty)

add_subdirectory(src)

target_include_directories(gegege_cpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_subdirectory(example)
