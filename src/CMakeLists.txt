add_library(gegege_cpp STATIC
    gegege/vulkan_engine/vulkan_engine.cpp
)

target_link_libraries(gegege_cpp PUBLIC
    Vulkan::Vulkan
    VulkanMemoryAllocator
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    stb
    lua
    glm
    imgui
)

add_executable(otsukimi WIN32
    gegege/otsukimi/main.cpp
)

set_target_properties(otsukimi PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(WIN32)
    add_custom_command(
        TARGET otsukimi POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:otsukimi>
        VERBATIM
    )
endif()

target_link_libraries(otsukimi PUBLIC gegege_cpp)
