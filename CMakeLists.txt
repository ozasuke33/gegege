cmake_minimum_required(VERSION 3.0)

project(gegege_proj LANGUAGES C CXX)

add_library(gegege_cpp
    src/gegege/vulkan_engine/vulkan_engine.cpp
)

target_include_directories(gegege_cpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_subdirectory(example)
