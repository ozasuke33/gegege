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
