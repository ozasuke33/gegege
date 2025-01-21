#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

#include <functional>
#include <vector>

namespace gegege::vulkan {

struct DeletionQueue {
    std::vector<std::function<void()>> deletors;

    void pushFunction(std::function<void()>&& function)
    {
        deletors.push_back(function);
    }

    void flush()
    {
        for (auto it = deletors.rbegin(); it != deletors.rend(); ++it)
        {
            (*it)();
        }
        deletors.clear();
    }
};

constexpr unsigned int FRAME_OVERLAP = 2;

struct FrameData {
    vk::CommandPool commandPool;
    vk::CommandBuffer mainCommandBuffer;
    vk::Semaphore swapchainSemaphore;
    vk::Semaphore renderSemaphore;
    vk::Fence renderFence;
    DeletionQueue deletionQueue;
};

struct AllocatedImage {
    vk::Image image;
    vk::ImageView imageView;
    VmaAllocation allocation;
    vk::Extent3D imageExtent;
    vk::Format imageFormat;
};

class VulkanEngine {

    vk::Extent2D windowExtent{640, 480};

    SDL_Window* sdlWindow;
    vk::Instance instance;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    uint32_t graphicsQueueFamilyIndex;
    uint32_t presentQueueFamilyIndex;
    vk::Queue graphicsQueue;
    vk::Queue presentQueue;
    vk::SurfaceKHR surface;
    vk::SwapchainKHR swapChain;
    std::vector<vk::Image> swapChainImages;
    std::vector<vk::ImageView> imageViews;

    AllocatedImage drawImage;
    vk::Extent2D drawExtent;

    FrameData frames[FRAME_OVERLAP];
    uint32_t frameNumber;
    FrameData& getCurrentFrame() { return frames[frameNumber % FRAME_OVERLAP]; }

    DeletionQueue mainDeletionQueue;

    VmaAllocator allocator;

    const uint64_t fenceTimeout = 100000000;

    bool stopRendering;

    vk::ImageCreateInfo imageCreateInfo(vk::Format format, vk::ImageUsageFlags usageFlags, vk::Extent3D extent);
    vk::ImageViewCreateInfo imageviewCreateInfo(vk::Format format, vk::Image image, vk::ImageAspectFlagBits aspectFlags);
    vk::ImageSubresourceRange imageSubresourceRange(vk::ImageAspectFlags aspectMask);
    void transitionImage(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout);
    void copyImageToImage(vk::CommandBuffer cmd, vk::Image source, vk::Image destination, vk::Extent2D srcSize, vk::Extent2D dstSize);
    void createSwapChain(uint32_t width, uint32_t height);
    void initSwapchain();
    void initCommandBuffer();
    void initSyncStructure();
    void initVulkan();
    void deinitVulkan();

    void drawBackground(vk::CommandBuffer cmd);

public:
    void startup();
    void draw();
    void run();
    void shutdown();
};

} // namespace gegege::vulkan
