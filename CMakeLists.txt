cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
project(gegege_proj LANGUAGES C CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(thirdparty)

add_subdirectory(src)

add_subdirectory(example)
