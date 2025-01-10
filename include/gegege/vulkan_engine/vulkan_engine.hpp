#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <vulkan/vulkan.hpp>

#include <vector>

namespace gegege {

class VulkanEngine {
    SDL_Window* sdlWindow;
    vk::Instance instance;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    uint32_t graphicsQueueFamilyIndex;
    uint32_t presentQueueFamilyIndex;
    vk::SurfaceKHR surface;
    vk::SwapchainKHR swapChain;
    std::vector<vk::ImageView> imageViews;

    void createSwapChain(uint32_t width, uint32_t height);
    void initVulkan();
    void deinitVulkan();

public:
    void startup();
    void shutdown();
};

} // namespace gegege
