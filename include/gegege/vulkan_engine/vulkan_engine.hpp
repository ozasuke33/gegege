#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <vulkan/vulkan.hpp>

#include <vector>

namespace gegege {

constexpr unsigned int FRAME_OVERLAP = 2;

struct FrameData {
    vk::CommandPool commandPool;
    vk::CommandBuffer mainCommandBuffer;
    vk::Semaphore swapchainSemaphore;
    vk::Semaphore renderSemaphore;
    vk::Fence renderFence;
};

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

    FrameData frames[FRAME_OVERLAP];
    uint32_t frameNumber;
    FrameData& getCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; }

    void createSwapChain(uint32_t width, uint32_t height);
    void initCommandBuffer();
    void initVulkan();
    void deinitVulkan();

public:
    void startup();
    void shutdown();
};

} // namespace gegege
