#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace gegege {

class VulkanEngine {
    SDL_Window* sdlWindow;

public:
    void startup();
    void shutdown();
};

} // namespace gegege
