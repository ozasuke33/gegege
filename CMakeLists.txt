cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
project(gegege_proj LANGUAGES C CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

find_package(Vulkan REQUIRED)

add_subdirectory(thirdparty)

add_subdirectory(src)

target_include_directories(gegege_cpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

set_target_properties(gegege_cpp PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
)

add_subdirectory(example)
