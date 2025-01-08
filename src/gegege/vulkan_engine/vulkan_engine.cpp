#include <gegege/vulkan_engine/vulkan_engine.hpp>

namespace gegege {

void VulkanEngine::startup()
{
    SDL_Log("Vulkan Engine: initialize SDL");
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Vulkan Engine: Couldn't initialize SDL: %s", SDL_GetError());
        abort();
    }

    SDL_Log("Vulkan Engine: create window");
    sdlWindow = SDL_CreateWindow("gegege::VulkanEngine", 640, 480, SDL_WINDOW_VULKAN);
    if (!sdlWindow)
    {
        SDL_Log("Vulkan Engine: Couldn't create window: %s", SDL_GetError());
    }
}

void VulkanEngine::shutdown()
{
    SDL_Log("Vulkan Engine: finalize SDL");
    SDL_Quit();
}

} // namespace gegege
