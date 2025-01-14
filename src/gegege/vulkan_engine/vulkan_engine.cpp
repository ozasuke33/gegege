#include <gegege/vulkan_engine/vulkan_engine.hpp>

namespace gegege {

vk::ImageSubresourceRange VulkanEngine::imageSubresourceRange(vk::ImageAspectFlags aspectMask)
{
    vk::ImageSubresourceRange subImage{};
    subImage.setAspectMask(aspectMask);
    subImage.setBaseMipLevel(0);
    subImage.setLevelCount(vk::RemainingMipLevels);
    subImage.setBaseArrayLayer(0);
    subImage.setLayerCount(vk::RemainingArrayLayers);

    return subImage;
}

void VulkanEngine::transitionImage(vk::CommandBuffer& cmd, vk::Image& image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout)
{
    vk::ImageMemoryBarrier2 imageBarrier{};

    imageBarrier.setSrcStageMask(vk::PipelineStageFlagBits2::eAllCommands);
    imageBarrier.setSrcAccessMask(vk::AccessFlagBits2::eMemoryWrite);
    imageBarrier.setDstStageMask(vk::PipelineStageFlagBits2::eAllCommands);
    imageBarrier.setDstAccessMask(vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead);

    imageBarrier.setOldLayout(currentLayout);
    imageBarrier.setNewLayout(newLayout);

    vk::ImageAspectFlagBits aspectMask = (newLayout == vk::ImageLayout::eDepthAttachmentOptimal) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
    imageBarrier.setSubresourceRange(imageSubresourceRange(aspectMask));
    imageBarrier.setImage(image);

    vk::DependencyInfo depInfo{};

    depInfo.setImageMemoryBarrierCount(1);
    depInfo.setPImageMemoryBarriers(&imageBarrier);

    cmd.pipelineBarrier2(depInfo);
}

void VulkanEngine::createSwapChain(uint32_t width, uint32_t height)
{
    SDL_Log("Vulkan Engine: create SwapChain");
    std::vector<vk::SurfaceFormatKHR> formats = physicalDevice.getSurfaceFormatsKHR(surface);
    assert(!formats.empty());
    vk::Format format = (formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0].format;

    vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    vk::Extent2D swapchainExtent;
    if (surfaceCapabilities.currentExtent.width == (std::numeric_limits<uint32_t>::max)())
    {
        // If the surface size is undefined, the size is set to the size of the images requested.
        swapchainExtent.width = SDL_clamp(width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
        swapchainExtent.height = SDL_clamp(height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
    }
    else
    {
        // If the surface size is defined, the swap chain size must match
        swapchainExtent = surfaceCapabilities.currentExtent;
    }

    // The FIFO present mode is guaranteed by the spec to be supported
    vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

    vk::SurfaceTransformFlagBitsKHR preTransform = (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
                                                       ? vk::SurfaceTransformFlagBitsKHR::eIdentity
                                                       : surfaceCapabilities.currentTransform;

    vk::CompositeAlphaFlagBitsKHR compositeAlpha =
        (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied)    ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
        : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
        : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit)        ? vk::CompositeAlphaFlagBitsKHR::eInherit
                                                                                                         : vk::CompositeAlphaFlagBitsKHR::eOpaque;

    vk::SwapchainCreateInfoKHR swapChainCreateInfo(vk::SwapchainCreateFlagsKHR(),
                                                   surface,
                                                   SDL_clamp(3u, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount),
                                                   format,
                                                   vk::ColorSpaceKHR::eSrgbNonlinear,
                                                   swapchainExtent,
                                                   1,
                                                   vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
                                                   vk::SharingMode::eExclusive,
                                                   {},
                                                   preTransform,
                                                   compositeAlpha,
                                                   swapchainPresentMode,
                                                   true,
                                                   nullptr);

    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
    {
        uint32_t queueFamilyIndices[2] = {graphicsQueueFamilyIndex, presentQueueFamilyIndex};
        // If the graphics and present queues are from different queue families, we either have to explicitly transfer
        // ownership of images between the queues, or we have to create the swapchain with imageSharingMode as
        // vk::SharingMode::eConcurrent
        swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    swapChain = device.createSwapchainKHR(swapChainCreateInfo);

    swapChainImages = device.getSwapchainImagesKHR(swapChain);

    imageViews.reserve(swapChainImages.size());
    vk::ImageViewCreateInfo imageViewCreateInfo({}, {}, vk::ImageViewType::e2D, format, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
    for (auto image : swapChainImages)
    {
        imageViewCreateInfo.image = image;
        imageViews.push_back(device.createImageView(imageViewCreateInfo));
    }
}

void VulkanEngine::initCommandBuffer()
{
    SDL_Log("Vulkan Engine: init command buffer");
    vk::CommandPoolCreateInfo createInfo{};
    createInfo.setQueueFamilyIndex(graphicsQueueFamilyIndex);
    createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

    for (int i = 0; i < FRAME_OVERLAP; ++i)
    {
        frames[i].commandPool = device.createCommandPool(createInfo);
        frames[i].mainCommandBuffer = device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(frames[i].commandPool, vk::CommandBufferLevel::ePrimary, 1)).front();
    }
}

void VulkanEngine::initSyncStructure()
{
    SDL_Log("Vulkan Engine: init sync structure");
    vk::FenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
    vk::SemaphoreCreateInfo semaphoreCreateInfo{};

    for (int i = 0; i < FRAME_OVERLAP; ++i)
    {
        frames[i].renderFence = device.createFence(fenceCreateInfo);

        frames[i].swapchainSemaphore = device.createSemaphore(semaphoreCreateInfo);
        frames[i].renderSemaphore = device.createSemaphore(semaphoreCreateInfo);
    }
}

void VulkanEngine::initVulkan()
{
    SDL_Log("Vulkan Engine: initialize Vulkan");

    std::vector<char const*> instanceLayerNames;
#if !defined(NDEBUG)
    instanceLayerNames.push_back("VK_LAYER_KHRONOS_validation");
#endif

    uint32_t count = 0;
    auto pExt = SDL_Vulkan_GetInstanceExtensions(&count);
    std::vector<const char*> ext(pExt, pExt + count);
    for (auto i : ext)
    {
        SDL_Log("Vulkan Engine: Vulkan Ext: %s", i);
    }

    vk::ApplicationInfo appInfo("", 1, "gegege", 1, VK_API_VERSION_1_3);
    vk::InstanceCreateInfo instanceCreateInfo({}, &appInfo, instanceLayerNames, ext);
    instance = vk::createInstance(instanceCreateInfo);

    if (!SDL_Vulkan_CreateSurface(sdlWindow, instance, nullptr, (VkSurfaceKHR*)&surface))
    {
        SDL_Log("Vulkan Engine: Couldn't create surface: %s", SDL_GetError());
        abort();
    }

    physicalDevice = instance.enumeratePhysicalDevices().front();

    SDL_Log("Vulkan Engine: device name: %s", physicalDevice.getProperties().deviceName);
    SDL_Log("Vulkan Engine: Vulkan version: %d.%d.%d", VK_API_VERSION_MAJOR(physicalDevice.getProperties().apiVersion), VK_API_VERSION_MINOR(physicalDevice.getProperties().apiVersion), VK_API_VERSION_PATCH(physicalDevice.getProperties().apiVersion));
    SDL_Log("Vulkan Engine: driver version: %d.%d.%d", VK_API_VERSION_MAJOR(physicalDevice.getProperties().driverVersion), VK_API_VERSION_MINOR(physicalDevice.getProperties().driverVersion), VK_API_VERSION_PATCH(physicalDevice.getProperties().driverVersion));

    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    auto propertyIterator = std::find_if(queueFamilyProperties.begin(),
                                         queueFamilyProperties.end(),
                                         [](vk::QueueFamilyProperties const& qfp) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; });
    assert(propertyIterator != queueFamilyProperties.end());
    graphicsQueueFamilyIndex = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), propertyIterator));

    presentQueueFamilyIndex = physicalDevice.getSurfaceSupportKHR(graphicsQueueFamilyIndex, surface)
                                  ? graphicsQueueFamilyIndex
                                  : queueFamilyProperties.size();
    if (presentQueueFamilyIndex == queueFamilyProperties.size())
    {
        for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
        {
            if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
                physicalDevice.getSurfaceSupportKHR(i, surface))
            {
                graphicsQueueFamilyIndex = i;
                presentQueueFamilyIndex = i;
                break;
            }
        }
        if (presentQueueFamilyIndex == queueFamilyProperties.size())
        {
            for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
            {
                if (physicalDevice.getSurfaceSupportKHR(i, surface))
                {
                    presentQueueFamilyIndex = i;
                    break;
                }
            }
        }
    }
    if ((graphicsQueueFamilyIndex == queueFamilyProperties.size()) || (presentQueueFamilyIndex == queueFamilyProperties.size()))
    {
        SDL_Log("Vulkan Engine: Couldn't find a queue for graphics or present");
        abort();
    }

    float queuePriority = 0.0f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), graphicsQueueFamilyIndex, 1, &queuePriority);
    std::vector<const char*> deviceExt = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::PhysicalDeviceVulkan13Features features{};
    features.setDynamicRendering(true);
    features.setSynchronization2(true);

    device = physicalDevice.createDevice(vk::DeviceCreateInfo(vk::DeviceCreateFlags(), deviceQueueCreateInfo, {}, deviceExt, {}, &features));

    graphicsQueue = device.getQueue(graphicsQueueFamilyIndex, 0);
    presentQueue = device.getQueue(presentQueueFamilyIndex, 0);

    createSwapChain(640, 480);

    initCommandBuffer();

    initSyncStructure();
}

void VulkanEngine::deinitVulkan()
{
    SDL_Log("Vulkan Engine: finalize Vulkan");
    device.waitIdle();

    for (int i = 0; i < FRAME_OVERLAP; ++i)
    {
        device.destroyCommandPool(frames[i].commandPool);

        device.destroyFence(frames[i].renderFence);
        device.destroySemaphore(frames[i].swapchainSemaphore);
        device.destroySemaphore(frames[i].renderSemaphore);

        frames[i].deletionQueue.flush();
    }

    mainDeletionQueue.flush();

    for (auto& imageView : imageViews)
    {
        device.destroyImageView(imageView);
    }
    imageViews.clear();
    device.destroySwapchainKHR(swapChain);
    SDL_Vulkan_DestroySurface(instance, surface, nullptr);
    device.destroy();
    instance.destroy();
}

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
        abort();
    }

    initVulkan();
}

void VulkanEngine::draw()
{
    device.waitForFences(getCurrentFrame().renderFence, true, fenceTimeout);
    device.resetFences(getCurrentFrame().renderFence);

    getCurrentFrame().deletionQueue.flush();

    vk::ResultValue<uint32_t> currentBuffer = device.acquireNextImageKHR(swapChain, fenceTimeout, getCurrentFrame().swapchainSemaphore, nullptr);
    assert(currentBuffer.result == vk::Result::eSuccess);
    uint32_t swapchainImageIndex = currentBuffer.value;

    vk::CommandBuffer cmd = getCurrentFrame().mainCommandBuffer;

    cmd.reset();

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    cmd.begin(beginInfo);

    // make the swapchain image into writeable mode before rendering
    transitionImage(cmd, swapChainImages[swapchainImageIndex], vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);

    // make a clear-color from frame number. This will flash with a 120 frame period.
    vk::ClearColorValue clearValue{};
    float flash = std::abs(std::sin(frameNumber / 120.0f));
    clearValue.setFloat32({0.0f, 0.0f, flash, 1.0f});

    vk::ImageSubresourceRange clearRange = imageSubresourceRange(vk::ImageAspectFlagBits::eColor);

    // clear image
    cmd.clearColorImage(swapChainImages[swapchainImageIndex], vk::ImageLayout::eGeneral, &clearValue, 1, &clearRange);

    // make the swapchain image into presentable mode
    transitionImage(cmd, swapChainImages[swapchainImageIndex], vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR);

    cmd.end();

    vk::CommandBufferSubmitInfo cmdInfo(cmd);

    vk::SemaphoreSubmitInfo waitInfo(getCurrentFrame().swapchainSemaphore, 1, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
    vk::SemaphoreSubmitInfo signalInfo(getCurrentFrame().renderSemaphore, 1, vk::PipelineStageFlagBits2::eAllGraphics);

    vk::SubmitInfo2 submit({}, {waitInfo}, {cmdInfo}, {signalInfo});

    graphicsQueue.submit2(1, &submit, getCurrentFrame().renderFence);

    vk::PresentInfoKHR presentInfo{};
    presentInfo.setSwapchains({swapChain});

    presentInfo.setWaitSemaphores({getCurrentFrame().renderSemaphore});

    presentInfo.setPImageIndices(&swapchainImageIndex);

    presentQueue.presentKHR(presentInfo);

    frameNumber++;
}

void VulkanEngine::run()
{
    SDL_Event ev;
    bool bQuit = false;

    while (!bQuit)
    {
        while (SDL_PollEvent(&ev) != 0)
        {
            if (ev.type == SDL_EVENT_QUIT)
            {
                SDL_Log("Vulkan Engine: SDL_EVENT_QUIT is occured");
                bQuit = true;
            }

            if (ev.type == SDL_EVENT_WINDOW_MINIMIZED)
            {
                SDL_Log("Vulkan Engine: SDL_EVENT_WINDOW_MINIMIZED is occured");
                stopRendering = true;
            }
            if (ev.type == SDL_EVENT_WINDOW_RESTORED)
            {
                SDL_Log("Vulkan Engine: SDL_EVENT_WINDOW_RESTORED is occured");
                stopRendering = false;
            }
        }

        if (stopRendering)
        {
            SDL_Delay(100);
        }
        else
        {
            draw();
        }
    }
}

void VulkanEngine::shutdown()
{
    deinitVulkan();

    SDL_Log("Vulkan Engine: finalize SDL");
    SDL_Quit();
}

} // namespace gegege
