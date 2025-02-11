#include <gegege/vulkan_engine/vk_check.hpp>
#include <gegege/vulkan_engine/vulkan_engine.hpp>
#include <gegege/assert.hpp>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include <vulkan/vulkan.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <fstream>

namespace gegege::vulkan {

vk::ImageCreateInfo VulkanEngine::imageCreateInfo(vk::Format format, vk::ImageUsageFlags usageFlags, vk::Extent3D extent)
{
    vk::ImageCreateInfo info{};

    info.setImageType(vk::ImageType::e2D);

    info.setFormat(format);
    info.setExtent(extent);

    info.setMipLevels(1);
    info.setArrayLayers(1);

    // for MSAA. we will not be using it by default, so default it to 1 sample per pixel.
    info.setSamples(vk::SampleCountFlagBits::e1);

    // optimal tiling, which means the image is stored on the best gpu format
    info.setTiling(vk::ImageTiling::eOptimal);
    info.setUsage(usageFlags);

    return info;
}

vk::ImageViewCreateInfo VulkanEngine::imageviewCreateInfo(vk::Format format, vk::Image image, vk::ImageAspectFlagBits aspectFlags)
{
    // build a image-view for the depth image to use for rendering
    vk::ImageViewCreateInfo info{};

    info.setViewType(vk::ImageViewType::e2D);
    info.setImage(image);
    info.setFormat(format);

    vk::ImageSubresourceRange range{};
    range.setBaseMipLevel(0);
    range.setLevelCount(1);
    range.setBaseArrayLayer(0);
    range.setLayerCount(1);
    range.setAspectMask(aspectFlags);

    info.setSubresourceRange(range);

    return info;
}

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

vk::RenderingAttachmentInfo VulkanEngine::attachmentInfo(vk::ImageView view, vk::ClearValue* clear, vk::ImageLayout layout)
{
    vk::RenderingAttachmentInfo colorAttachment{};

    colorAttachment.setImageView(view);
    colorAttachment.setImageLayout(layout);
    colorAttachment.setLoadOp(clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eLoad);
    colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
    if (clear)
    {
        colorAttachment.setClearValue(*clear);
    }

    return colorAttachment;
}

vk::RenderingInfo VulkanEngine::renderingInfo(vk::Extent2D renderExtent, vk::RenderingAttachmentInfo* colorAttachment, vk::RenderingAttachmentInfo* depthAttachment)
{
    vk::RenderingInfo renderInfo{};

    renderInfo.setRenderArea(vk::Rect2D{vk::Offset2D{0, 0}, renderExtent});
    renderInfo.setLayerCount(1);
    renderInfo.setColorAttachments({*colorAttachment});
    renderInfo.setPDepthAttachment(depthAttachment);

    return renderInfo;
}

void VulkanEngine::loadShaderModule(const char* filePath, vk::Device device, vk::ShaderModule* outShaderModule)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        SDL_Log("Vulkan Engine: Failed to load file: %s", filePath);
    }
    assert(file.is_open());

    // find what the size of the file is by looking up the location of the cursor
    // because the cursor is at the end, it gives the size directly in bytes
    size_t fileSize = (size_t)file.tellg();

    // spirv expects the buffer to be on uint32, so make sure to reserve a int
    // vector big enough for the entire file
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    // put file cursor at beginning
    file.seekg(0);

    // load the entire file into the buffer
    file.read((char*)buffer.data(), fileSize);

    // now that the file is loaded into the buffer, we can close it
    file.close();

    vk::ShaderModuleCreateInfo createInfo{};

    // codeSize has to be in bytes, so multply the ints in the buffer by size of
    // int to know the real size of the buffer
    createInfo.setCodeSize(buffer.size() * sizeof(uint32_t));
    createInfo.setPCode(buffer.data());

    vk::ShaderModule shaderModule{};

    VK_CHECK(device.createShaderModule(&createInfo, nullptr, &shaderModule));

    *outShaderModule = shaderModule;
}

void VulkanEngine::transitionImage(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout)
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

void VulkanEngine::copyImageToImage(vk::CommandBuffer cmd, vk::Image source, vk::Image destination, vk::Extent2D srcSize, vk::Extent2D dstSize)
{
    vk::ImageBlit2 blitRegion{};

    blitRegion.srcOffsets[1].setX(srcSize.width);
    blitRegion.srcOffsets[1].setY(srcSize.height);
    blitRegion.srcOffsets[1].setZ(1);

    blitRegion.dstOffsets[1].setX(dstSize.width);
    blitRegion.dstOffsets[1].setY(dstSize.height);
    blitRegion.dstOffsets[1].setZ(1);

    blitRegion.srcSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
    blitRegion.srcSubresource.setBaseArrayLayer(0);
    blitRegion.srcSubresource.setLayerCount(1);
    blitRegion.srcSubresource.setMipLevel(0);

    blitRegion.dstSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
    blitRegion.dstSubresource.setBaseArrayLayer(0);
    blitRegion.dstSubresource.setLayerCount(1);
    blitRegion.dstSubresource.setMipLevel(0);

    vk::BlitImageInfo2 blitInfo{};
    blitInfo.setDstImage(destination);
    blitInfo.setDstImageLayout(vk::ImageLayout::eTransferDstOptimal);
    blitInfo.setSrcImage(source);
    blitInfo.setSrcImageLayout(vk::ImageLayout::eTransferSrcOptimal);
    blitInfo.setFilter(vk::Filter::eLinear);
    blitInfo.setRegionCount(1);
    blitInfo.setPRegions(&blitRegion);

    cmd.blitImage2(&blitInfo);
}

void VulkanEngine::immediate_submit(std::function<void(vk::CommandBuffer cmd)>&& function)
{
    VK_CHECK(mDevice.resetFences(1, &mImmFence));
    mImmCommandBuffer.reset();

    vk::CommandBuffer cmd = mImmCommandBuffer;

    vk::CommandBufferBeginInfo cmdBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    VK_CHECK(cmd.begin(&cmdBeginInfo));

    function(cmd);

    cmd.end();

    vk::CommandBufferSubmitInfo cmdInfo(cmd);
    vk::SubmitInfo2 submit({}, {}, {cmdInfo});

    VK_CHECK(mGraphicsQueue.submit2(1, &submit, mImmFence));
    VK_CHECK(mDevice.waitForFences(1, &mImmFence, true, 9999999999));
}

void VulkanEngine::createSwapChain(uint32_t width, uint32_t height)
{
    SDL_Log("Vulkan Engine: create SwapChain");
    std::vector<vk::SurfaceFormatKHR> formats = mPhysicalDevice.getSurfaceFormatsKHR(mSurface);
    assert(!formats.empty());
    vk::Format format = (formats[0].format == vk::Format::eUndefined) ? vk::Format::eB8G8R8A8Unorm : formats[0].format;

    vk::SurfaceCapabilitiesKHR surfaceCapabilities = mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface);
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
                                                   mSurface,
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

    if (mGraphicsQueueFamilyIndex != mPresentQueueFamilyIndex)
    {
        uint32_t queueFamilyIndices[2] = {mGraphicsQueueFamilyIndex, mPresentQueueFamilyIndex};
        // If the graphics and present queues are from different queue families, we either have to explicitly transfer
        // ownership of images between the queues, or we have to create the swapchain with imageSharingMode as
        // vk::SharingMode::eConcurrent
        swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    mSwapchain = mDevice.createSwapchainKHR(swapChainCreateInfo);

    mSwapchainImages = mDevice.getSwapchainImagesKHR(mSwapchain);

    mSwapchainImageViews.reserve(mSwapchainImages.size());
    vk::ImageViewCreateInfo imageViewCreateInfo({}, {}, vk::ImageViewType::e2D, format, {}, {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});
    for (auto image : mSwapchainImages)
    {
        imageViewCreateInfo.image = image;
        mSwapchainImageViews.push_back(mDevice.createImageView(imageViewCreateInfo));
    }

    mSwapchainImageFormat = format;
}

void VulkanEngine::initSwapchain()
{
    createSwapChain(mWindowExtent.width, mWindowExtent.height);

    vk::Extent3D drawImageExtent = {
        mWindowExtent.width,
        mWindowExtent.height,
        1};

    mDrawImage.mImageFormat = vk::Format::eR16G16B16A16Sfloat;
    mDrawImage.mImageExtent = drawImageExtent;

    vk::ImageUsageFlags drawImageUsages{};
    drawImageUsages |= vk::ImageUsageFlagBits::eTransferSrc;
    drawImageUsages |= vk::ImageUsageFlagBits::eTransferDst;
    drawImageUsages |= vk::ImageUsageFlagBits::eStorage;
    drawImageUsages |= vk::ImageUsageFlagBits::eColorAttachment;

    vk::ImageCreateInfo rimgInfo = imageCreateInfo(mDrawImage.mImageFormat, drawImageUsages, drawImageExtent);

    // for the draw image, we want to allocate it from gpu local memory
    VmaAllocationCreateInfo rimgAllocInfo{};
    rimgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rimgAllocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    // allocate and create the image
    vmaCreateImage(mAllocator, (const VkImageCreateInfo*)(&rimgInfo), (const VmaAllocationCreateInfo*)(&rimgAllocInfo), (VkImage*)&mDrawImage.mImage, &mDrawImage.mAllocation, nullptr);

    // build a mImage-view for the draw mImage to use for rendering
    vk::ImageViewCreateInfo rviewInfo = imageviewCreateInfo(mDrawImage.mImageFormat, mDrawImage.mImage, vk::ImageAspectFlagBits::eColor);

    mDrawImage.mImageView = mDevice.createImageView(rviewInfo);

    mMainDeletionQueue.pushFunction([=, this]() {
        mDevice.destroyImageView(mDrawImage.mImageView);
        vmaDestroyImage(mAllocator, mDrawImage.mImage, mDrawImage.mAllocation);
    });
}

void VulkanEngine::initCommandBuffer()
{
    SDL_Log("Vulkan Engine: init command buffer");
    vk::CommandPoolCreateInfo createInfo{};
    createInfo.setQueueFamilyIndex(mGraphicsQueueFamilyIndex);
    createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

    for (int i = 0; i < FRAME_OVERLAP; ++i)
    {
        mFrames[i].mCommandPool = mDevice.createCommandPool(createInfo);
        mFrames[i].mMainCommandBuffer = mDevice.allocateCommandBuffers(vk::CommandBufferAllocateInfo(mFrames[i].mCommandPool, vk::CommandBufferLevel::ePrimary, 1)).front();
    }

    mImmCommandPool = mDevice.createCommandPool(createInfo);

    mImmCommandBuffer = mDevice.allocateCommandBuffers(vk::CommandBufferAllocateInfo(mImmCommandPool, vk::CommandBufferLevel::ePrimary, 1)).front();

    mMainDeletionQueue.pushFunction([=, this]() {
        mDevice.destroyCommandPool(mImmCommandPool);
    });
}

void VulkanEngine::initSyncStructure()
{
    SDL_Log("Vulkan Engine: init sync structure");
    vk::FenceCreateInfo fenceCreateInfo;
    fenceCreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);
    vk::SemaphoreCreateInfo semaphoreCreateInfo{};

    for (int i = 0; i < FRAME_OVERLAP; ++i)
    {
        mFrames[i].mRenderFence = mDevice.createFence(fenceCreateInfo);

        mFrames[i].mSwapchainSemaphore = mDevice.createSemaphore(semaphoreCreateInfo);
        mFrames[i].mRenderSemaphore = mDevice.createSemaphore(semaphoreCreateInfo);
    }

    mImmFence = mDevice.createFence(fenceCreateInfo);
    mMainDeletionQueue.pushFunction([=, this]() { mDevice.destroyFence(mImmFence); });
}

void VulkanEngine::initDescriptor()
{
    SDL_Log("Vulkan Engine: init descriptor");
    std::vector<DescriptorAllocator::PoolSizeRatio> sizes{{vk::DescriptorType::eStorageImage, 1.0f}};

    mGlobalDescriptorAllocator.initPool(mDevice, 10, sizes);

    {
        DescriptorLayoutBuilder builder{};
        builder.add_binding(0, vk::DescriptorType::eStorageImage);
        mDrawImageDescriptorLayout = builder.build(mDevice, vk::ShaderStageFlagBits::eCompute);
    }

    mDrawImageDescriptors = mGlobalDescriptorAllocator.allocate(mDevice, mDrawImageDescriptorLayout);

    vk::DescriptorImageInfo imgInfo{};
    imgInfo.setImageLayout(vk::ImageLayout::eGeneral);
    imgInfo.setImageView(mDrawImage.mImageView);

    vk::WriteDescriptorSet drawImageWrite{};

    drawImageWrite.setDstBinding(0);
    drawImageWrite.setDstSet(mDrawImageDescriptors);
    drawImageWrite.setDescriptorCount(1);
    drawImageWrite.setDescriptorType(vk::DescriptorType::eStorageImage);
    drawImageWrite.setPImageInfo(&imgInfo);

    mDevice.updateDescriptorSets(1, &drawImageWrite, 0, nullptr);

    mMainDeletionQueue.pushFunction([&]() {
        mGlobalDescriptorAllocator.destroyPool(mDevice);

        mDevice.destroyDescriptorSetLayout(mDrawImageDescriptorLayout);
    });
}

void VulkanEngine::initPipeline()
{
    SDL_Log("Vulkan Engine: init pipeline");
    initBackgroundPipeline();
}

void VulkanEngine::initBackgroundPipeline()
{
    vk::PipelineLayoutCreateInfo computeLayout{};
    computeLayout.setPSetLayouts(&mDrawImageDescriptorLayout);
    computeLayout.setSetLayoutCount(1);

    vk::PushConstantRange pushConstant{};
    pushConstant.setSize(sizeof(ComputePushConstants));
    pushConstant.setStageFlags(vk::ShaderStageFlagBits::eCompute);

    computeLayout.setPushConstantRanges({pushConstant});

    mGradientPipelineLayout = mDevice.createPipelineLayout(computeLayout);

    vk::ShaderModule gradientShader{};
    loadShaderModule("../../../../shaders/gradient_color.comp.spv", mDevice, &gradientShader);

    vk::ShaderModule skyShader{};
    loadShaderModule("../../../../shaders/sky.comp.spv", mDevice, &skyShader);

    vk::PipelineShaderStageCreateInfo stageInfo{};
    stageInfo.setStage(vk::ShaderStageFlagBits::eCompute);
    stageInfo.setModule(gradientShader);
    stageInfo.setPName("main");

    vk::ComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.setLayout(mGradientPipelineLayout);
    computePipelineCreateInfo.setStage(stageInfo);

    ComputeEffect gradient{};
    gradient.mLayout = mGradientPipelineLayout;
    gradient.mName = "gradient";
    gradient.mData = {};

    // default colors
    gradient.mData.mData1 = glm::vec4(1, 0, 0, 1);
    gradient.mData.mData2 = glm::vec4(0, 0, 1, 1);

    gradient.mPipeline = mDevice.createComputePipelines(VK_NULL_HANDLE, {computePipelineCreateInfo}).value.front();

    // change the shader module only to create the sky shader
    computePipelineCreateInfo.stage.module = skyShader;

    ComputeEffect sky;
    sky.mLayout = mGradientPipelineLayout;
    sky.mName = "sky";
    sky.mData = {};
    // default sky parameters
    sky.mData.mData1 = glm::vec4(0.1, 0.2, 0.4, 0.97);

    sky.mPipeline = mDevice.createComputePipelines(VK_NULL_HANDLE, {computePipelineCreateInfo}).value.front();

    // add the 2 background effects into the array
    mBackgroundEffects.push_back(gradient);
    mBackgroundEffects.push_back(sky);

    mDevice.destroyShaderModule(gradientShader);
    mDevice.destroyShaderModule(skyShader);

    mMainDeletionQueue.pushFunction([=, this]() {
        mDevice.destroyPipelineLayout(mGradientPipelineLayout);
        mDevice.destroyPipeline(sky.mPipeline);
        mDevice.destroyPipeline(gradient.mPipeline);
    });
}

void VulkanEngine::initImGui()
{
    SDL_Log("Vulkan Engine: init ImGui");
    // 1: create descriptor pool for IMGUI
    //  the size of the pool is very oversize, but it's copied from imgui demo
    //  itself.
    vk::DescriptorPoolSize poolSizes[11];
    poolSizes[0] = {VK_DESCRIPTOR_TYPE_SAMPLER, 1000};
    poolSizes[1] = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000};
    poolSizes[2] = {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000};
    poolSizes[3] = {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000};
    poolSizes[4] = {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000};
    poolSizes[5] = {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000};
    poolSizes[6] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000};
    poolSizes[7] = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000};
    poolSizes[8] = {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000};
    poolSizes[9] = {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000};
    poolSizes[10] = {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000};

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
    poolInfo.setMaxSets(1000);
    poolInfo.setPoolSizes(poolSizes);

    vk::DescriptorPool imguiPool{};
    imguiPool = mDevice.createDescriptorPool(poolInfo);

    // 2: initialize imgui library

    // this initializes the core structures of imgui
    ImGui::CreateContext();

    // this initializes imgui for SDL
    ImGui_ImplSDL3_InitForVulkan(mSdlWindow);

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = mInstance;
    initInfo.PhysicalDevice = mPhysicalDevice;
    initInfo.Device = mDevice;
    initInfo.Queue = mGraphicsQueue;
    initInfo.DescriptorPool = imguiPool;
    initInfo.MinImageCount = 3;
    initInfo.ImageCount = 3;
    initInfo.UseDynamicRendering = true;

    // dynamic rendering parameters for imgui to use
    initInfo.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    initInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    initInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = (const VkFormat*)&mSwapchainImageFormat;

    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);

    ImGui_ImplVulkan_CreateFontsTexture();

    // add the destroy the imgui created structures
    mMainDeletionQueue.pushFunction([=, this]() {
        ImGui_ImplVulkan_Shutdown();
        mDevice.destroyDescriptorPool(imguiPool);
    });
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
    mInstance = vk::createInstance(instanceCreateInfo);

    if (!SDL_Vulkan_CreateSurface(mSdlWindow, mInstance, nullptr, (VkSurfaceKHR*)&mSurface))
    {
        SDL_Log("Vulkan Engine: Couldn't create surface: %s", SDL_GetError());
        abort();
    }

    mPhysicalDevice = mInstance.enumeratePhysicalDevices().front();

    SDL_Log("Vulkan Engine: device name: %s", mPhysicalDevice.getProperties().deviceName);
    SDL_Log("Vulkan Engine: Vulkan version: %d.%d.%d", VK_API_VERSION_MAJOR(mPhysicalDevice.getProperties().apiVersion), VK_API_VERSION_MINOR(mPhysicalDevice.getProperties().apiVersion), VK_API_VERSION_PATCH(mPhysicalDevice.getProperties().apiVersion));
    SDL_Log("Vulkan Engine: driver version: %d.%d.%d", VK_API_VERSION_MAJOR(mPhysicalDevice.getProperties().driverVersion), VK_API_VERSION_MINOR(mPhysicalDevice.getProperties().driverVersion), VK_API_VERSION_PATCH(mPhysicalDevice.getProperties().driverVersion));

    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = mPhysicalDevice.getQueueFamilyProperties();

    auto propertyIterator = std::find_if(queueFamilyProperties.begin(),
                                         queueFamilyProperties.end(),
                                         [](vk::QueueFamilyProperties const& qfp) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; });
    assert(propertyIterator != queueFamilyProperties.end());
    mGraphicsQueueFamilyIndex = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), propertyIterator));

    mPresentQueueFamilyIndex = mPhysicalDevice.getSurfaceSupportKHR(mGraphicsQueueFamilyIndex, mSurface)
                                  ? mGraphicsQueueFamilyIndex
                                  : queueFamilyProperties.size();
    if (mPresentQueueFamilyIndex == queueFamilyProperties.size())
    {
        for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
        {
            if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
                mPhysicalDevice.getSurfaceSupportKHR(i, mSurface))
            {
                mGraphicsQueueFamilyIndex = i;
                mPresentQueueFamilyIndex = i;
                break;
            }
        }
        if (mPresentQueueFamilyIndex == queueFamilyProperties.size())
        {
            for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
            {
                if (mPhysicalDevice.getSurfaceSupportKHR(i, mSurface))
                {
                    mPresentQueueFamilyIndex = i;
                    break;
                }
            }
        }
    }
    if ((mGraphicsQueueFamilyIndex == queueFamilyProperties.size()) || (mPresentQueueFamilyIndex == queueFamilyProperties.size()))
    {
        SDL_Log("Vulkan Engine: Couldn't find a queue for graphics or present");
        abort();
    }

    float queuePriority = 0.0f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), mGraphicsQueueFamilyIndex, 1, &queuePriority);
    std::vector<const char*> deviceExt = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::PhysicalDeviceVulkan12Features features12{};
    features12.setBufferDeviceAddress(true);

    vk::PhysicalDeviceVulkan13Features features13{};
    features13.setDynamicRendering(true);
    features13.setSynchronization2(true);

    features12.setPNext(&features13);

    mDevice = mPhysicalDevice.createDevice(vk::DeviceCreateInfo(vk::DeviceCreateFlags(), deviceQueueCreateInfo, {}, deviceExt, {}, &features12));

    mGraphicsQueue = mDevice.getQueue(mGraphicsQueueFamilyIndex, 0);
    mPresentQueue = mDevice.getQueue(mPresentQueueFamilyIndex, 0);

    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.physicalDevice = mPhysicalDevice;
    allocatorInfo.device = mDevice;
    allocatorInfo.instance = mInstance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &mAllocator);

    mMainDeletionQueue.pushFunction([&]() {
        vmaDestroyAllocator(mAllocator);
    });

    initSwapchain();

    initCommandBuffer();

    initSyncStructure();

    initDescriptor();

    initPipeline();

    initImGui();
}

void VulkanEngine::deinitVulkan()
{
    SDL_Log("Vulkan Engine: finalize Vulkan");
    mDevice.waitIdle();

    for (int i = 0; i < FRAME_OVERLAP; ++i)
    {
        mDevice.destroyCommandPool(mFrames[i].mCommandPool);

        mDevice.destroyFence(mFrames[i].mRenderFence);
        mDevice.destroySemaphore(mFrames[i].mSwapchainSemaphore);
        mDevice.destroySemaphore(mFrames[i].mRenderSemaphore);

        mFrames[i].mDeletionQueue.flush();
    }

    mMainDeletionQueue.flush();

    for (auto& imageView : mSwapchainImageViews)
    {
        mDevice.destroyImageView(imageView);
    }
    mSwapchainImageViews.clear();
    mDevice.destroySwapchainKHR(mSwapchain);
    SDL_Vulkan_DestroySurface(mInstance, mSurface, nullptr);
    mDevice.destroy();
    mInstance.destroy();
}

void VulkanEngine::drawBackground(vk::CommandBuffer cmd)
{
    ComputeEffect& effect = mBackgroundEffects[mCurrentBackgroundEffect];

    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, effect.mPipeline);

    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mGradientPipelineLayout, 0, 1, &mDrawImageDescriptors, 0, nullptr);

    cmd.pushConstants(mGradientPipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(ComputePushConstants), &effect.mData);

    cmd.dispatch(std::ceil(mDrawExtent.width / 16.0f), std::ceil(mDrawExtent.height / 16.0f), 1);
}

void VulkanEngine::drawImGui(vk::CommandBuffer cmd, vk::ImageView targetImageView)
{
    vk::RenderingAttachmentInfo colorAttachment = attachmentInfo(targetImageView, nullptr);
    vk::RenderingInfo renderInfo = renderingInfo(mWindowExtent, &colorAttachment, nullptr);

    cmd.beginRendering(&renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    cmd.endRendering();
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
    mSdlWindow = SDL_CreateWindow("gegege::VulkanEngine", mWindowExtent.width, mWindowExtent.height, SDL_WINDOW_VULKAN);
    if (!mSdlWindow)
    {
        SDL_Log("Vulkan Engine: Couldn't create window: %s", SDL_GetError());
        abort();
    }

    initVulkan();
}

void VulkanEngine::draw()
{
    VK_CHECK(mDevice.waitForFences(getCurrentFrame().mRenderFence, true, mFenceTimeout));
    mDevice.resetFences(getCurrentFrame().mRenderFence);

    getCurrentFrame().mDeletionQueue.flush();

    vk::ResultValue<uint32_t> currentBuffer = mDevice.acquireNextImageKHR(mSwapchain, mFenceTimeout, getCurrentFrame().mSwapchainSemaphore, nullptr);
    assert(currentBuffer.result == vk::Result::eSuccess);
    uint32_t swapchainImageIndex = currentBuffer.value;

    vk::CommandBuffer cmd = getCurrentFrame().mMainCommandBuffer;

    cmd.reset();

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    mDrawExtent.width = mDrawImage.mImageExtent.width;
    mDrawExtent.height = mDrawImage.mImageExtent.height;

    cmd.begin(beginInfo);

    // transition our main draw image into general layout so we can write into it
    // we will overwrite it all so we dont care about what was the older layout
    transitionImage(cmd, mDrawImage.mImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);

    drawBackground(cmd);

    // transition the draw image and the swapchain image into their correct transfer layouts
    transitionImage(cmd, mDrawImage.mImage, vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal);
    transitionImage(cmd, mSwapchainImages[swapchainImageIndex], vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    copyImageToImage(cmd, mDrawImage.mImage, mSwapchainImages[swapchainImageIndex], mDrawExtent, mWindowExtent);

    // set swapchain image layout to Attachment Optimal so we can draw it
    transitionImage(cmd, mSwapchainImages[swapchainImageIndex], vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eColorAttachmentOptimal);

    drawImGui(cmd, mSwapchainImageViews[swapchainImageIndex]);

    // set swapchain image layout to Present so we can show it on the screen
    transitionImage(cmd, mSwapchainImages[swapchainImageIndex], vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR);

    cmd.end();

    vk::CommandBufferSubmitInfo cmdInfo(cmd);

    vk::SemaphoreSubmitInfo waitInfo(getCurrentFrame().mSwapchainSemaphore, 1, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
    vk::SemaphoreSubmitInfo signalInfo(getCurrentFrame().mRenderSemaphore, 1, vk::PipelineStageFlagBits2::eAllGraphics);

    vk::SubmitInfo2 submit({}, {waitInfo}, {cmdInfo}, {signalInfo});

    VK_CHECK(mGraphicsQueue.submit2(1, &submit, getCurrentFrame().mRenderFence));

    vk::PresentInfoKHR presentInfo{};
    presentInfo.setSwapchains({mSwapchain});

    presentInfo.setWaitSemaphores({getCurrentFrame().mRenderSemaphore});

    presentInfo.setPImageIndices(&swapchainImageIndex);

    VK_CHECK(mPresentQueue.presentKHR(presentInfo));

    mFrameNumber++;
}

void VulkanEngine::run()
{
    SDL_Event e;
    bool bQuit = false;

    while (!bQuit)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_EVENT_QUIT)
            {
                SDL_Log("Vulkan Engine: SDL_EVENT_QUIT is occured");
                bQuit = true;
            }

            if (e.type == SDL_EVENT_WINDOW_MINIMIZED)
            {
                SDL_Log("Vulkan Engine: SDL_EVENT_WINDOW_MINIMIZED is occured");
                mStopRendering = true;
            }
            if (e.type == SDL_EVENT_WINDOW_RESTORED)
            {
                SDL_Log("Vulkan Engine: SDL_EVENT_WINDOW_RESTORED is occured");
                mStopRendering = false;
            }

            ImGui_ImplSDL3_ProcessEvent(&e);
        }

        if (mStopRendering)
        {
            SDL_Delay(100);
            continue;
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        if (ImGui::Begin("background"))
        {
            ComputeEffect& selected = mBackgroundEffects[mCurrentBackgroundEffect];

            ImGui::Text("Selected effect: ", selected.mName);

            ImGui::SliderInt("Effect index", &mCurrentBackgroundEffect, 0, mBackgroundEffects.size() - 1);

            ImGui::InputFloat4("data1", (float*)&selected.mData.mData1);
            ImGui::InputFloat4("data2", (float*)&selected.mData.mData2);
            ImGui::InputFloat4("data3", (float*)&selected.mData.mData3);
            ImGui::InputFloat4("data4", (float*)&selected.mData.mData4);
        }
        ImGui::End();

        ImGui::Render();

        draw();
    }
}

void VulkanEngine::shutdown()
{
    deinitVulkan();

    SDL_Log("Vulkan Engine: finalize SDL");
    SDL_Quit();
}

} // namespace gegege::vulkan
