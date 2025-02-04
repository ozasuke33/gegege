#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

#include <glm/glm.hpp>

#include <functional>
#include <vector>

#include "descriptor.hpp"

namespace gegege::vulkan {

struct DeletionQueue {
    std::vector<std::function<void()>> mDeletors;

    void pushFunction(std::function<void()>&& function)
    {
        mDeletors.push_back(function);
    }

    void flush()
    {
        for (auto it = mDeletors.rbegin(); it != mDeletors.rend(); ++it)
        {
            (*it)();
        }
        mDeletors.clear();
    }
};

constexpr unsigned int FRAME_OVERLAP = 2;

struct FrameData {
    vk::CommandPool mCommandPool;
    vk::CommandBuffer mMainCommandBuffer;
    vk::Semaphore mSwapchainSemaphore;
    vk::Semaphore mRenderSemaphore;
    vk::Fence mRenderFence;
    DeletionQueue mDeletionQueue;
};

struct ComputePushConstants {
    glm::vec4 mData1;
    glm::vec4 mData2;
    glm::vec4 mData3;
    glm::vec4 mData4;
};

struct ComputeEffect {
    const char* mName;

    VkPipeline mPipeline;
    VkPipelineLayout mLayout;

    ComputePushConstants mData;
};

struct AllocatedImage {
    vk::Image mImage;
    vk::ImageView mImageView;
    VmaAllocation mAllocation;
    vk::Extent3D mImageExtent;
    vk::Format mImageFormat;
};

class VulkanEngine {

    vk::Extent2D mWindowExtent{640, 480};

    SDL_Window* mSdlWindow;
    vk::Instance mInstance;
    vk::PhysicalDevice mPhysicalDevice;
    vk::Device mDevice;
    uint32_t mGraphicsQueueFamilyIndex;
    uint32_t mPresentQueueFamilyIndex;
    vk::Queue mGraphicsQueue;
    vk::Queue mPresentQueue;
    vk::SurfaceKHR mSurface;
    vk::SwapchainKHR mSwapchain;
    vk::Format mSwapchainImageFormat;
    std::vector<vk::Image> mSwapchainImages;
    std::vector<vk::ImageView> mSwapchainImageViews;

    DescriptorAllocator mGlobalDescriptorAllocator;

    vk::DescriptorSet mDrawImageDescriptors;
    vk::DescriptorSetLayout mDrawImageDescriptorLayout;

    vk::PipelineLayout mGradientPipelineLayout;

    vk::Fence mImmFence;
    vk::CommandBuffer mImmCommandBuffer;
    vk::CommandPool mImmCommandPool;

    AllocatedImage mDrawImage;
    vk::Extent2D mDrawExtent;

    FrameData mFrames[FRAME_OVERLAP];
    uint32_t mFrameNumber;
    FrameData& getCurrentFrame() { return mFrames[mFrameNumber % FRAME_OVERLAP]; }

    std::vector<ComputeEffect> mBackgroundEffects;
    int mCurrentBackgroundEffect{0};

    DeletionQueue mMainDeletionQueue;

    VmaAllocator mAllocator;

    const uint64_t mFenceTimeout = 100000000;

    bool mStopRendering;

    vk::ImageCreateInfo imageCreateInfo(vk::Format format, vk::ImageUsageFlags usageFlags, vk::Extent3D extent);
    vk::ImageViewCreateInfo imageviewCreateInfo(vk::Format format, vk::Image image, vk::ImageAspectFlagBits aspectFlags);
    vk::ImageSubresourceRange imageSubresourceRange(vk::ImageAspectFlags aspectMask);
    vk::RenderingAttachmentInfo attachmentInfo(vk::ImageView view, vk::ClearValue* clear, vk::ImageLayout layout = vk::ImageLayout::eColorAttachmentOptimal);
    vk::RenderingInfo renderingInfo(vk::Extent2D renderExtent, vk::RenderingAttachmentInfo* colorAttachment, vk::RenderingAttachmentInfo* depthAttachment);

    void loadShaderModule(const char* filePath, vk::Device device, vk::ShaderModule* outShaderModule);

    void transitionImage(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout);
    void copyImageToImage(vk::CommandBuffer cmd, vk::Image source, vk::Image destination, vk::Extent2D srcSize, vk::Extent2D dstSize);

    void immediate_submit(std::function<void(vk::CommandBuffer cmd)>&& function);

    void createSwapChain(uint32_t width, uint32_t height);
    void initSwapchain();
    void initCommandBuffer();
    void initSyncStructure();
    void initDescriptor();
    void initPipeline();
    void initBackgroundPipeline();
    void initImGui();
    void initVulkan();
    void deinitVulkan();

    void drawBackground(vk::CommandBuffer cmd);
    void drawImGui(vk::CommandBuffer cmd, vk::ImageView targetImageView);

public:
    void startup();
    void draw();
    void run();
    void shutdown();
};

} // namespace gegege::vulkan
