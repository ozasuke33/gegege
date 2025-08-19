add_library(gegege_cpp
    gegege/vulkan_engine/vulkan_engine.cpp
)

target_link_libraries(gegege_cpp PUBLIC
    Vulkan::Vulkan
    VulkanMemoryAllocator
    SDL3::SDL3
    lua
    glm
    imgui
)

add_executable(otsukimi
    gegege/otsukimi/main.cpp
)

set_target_properties(otsukimi PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(otsukimi PUBLIC gegege_cpp)
