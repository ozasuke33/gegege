#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

#include <gegege/vulkan_engine/vk_check.hpp>

namespace gegege::vulkan {


class PipelineBuilder {

    std::vector<vk::PipelineShaderStageCreateInfo> mShaderStages;

    vk::PipelineInputAssemblyStateCreateInfo mInputAssembly;
    vk::PipelineRasterizationStateCreateInfo mRasterizer;
    vk::PipelineColorBlendAttachmentState mColorBlendAttachment;
    vk::PipelineMultisampleStateCreateInfo mMultisampling;
    vk::PipelineLayout mPipelineLayout;
    vk::PipelineDepthStencilStateCreateInfo mDepthStencil;
    vk::PipelineRenderingCreateInfo mRenderInfo;
    vk::Format mColorAttachmentFormat;

    void clear()
    {
        mShaderStages.clear();

        mInputAssembly = vk::PipelineInputAssemblyStateCreateInfo{};
        mRasterizer = vk::PipelineRasterizationStateCreateInfo{};
        mColorBlendAttachment = vk::PipelineColorBlendAttachmentState{};
        mMultisampling = vk::PipelineMultisampleStateCreateInfo{};
        mPipelineLayout = vk::PipelineLayout{};
        mDepthStencil = vk::PipelineDepthStencilStateCreateInfo{};
        mRenderInfo = vk::PipelineRenderingCreateInfo{};
        mColorAttachmentFormat = vk::Format{};
    }

    vk::Pipeline buildPipeline(vk::Device device)
    {
        vk::PipelineViewportStateCreateInfo viewportState{};

        viewportState.setViewportCount(1);
        viewportState.setScissorCount(1);

        vk::PipelineColorBlendStateCreateInfo colorBlending{};

        colorBlending.setLogicOpEnable(vk::False);
        colorBlending.setLogicOp(vk::LogicOp::eCopy);
        colorBlending.setAttachments({mColorBlendAttachment});

        vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

        vk::GraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.setPNext(&mRenderInfo);

        pipelineInfo.setStages(mShaderStages);
        pipelineInfo.setPVertexInputState(&vertexInputInfo);
        pipelineInfo.setPInputAssemblyState(&mInputAssembly);
        pipelineInfo.setPViewportState(&viewportState);
        pipelineInfo.setPRasterizationState(&mRasterizer);
        pipelineInfo.setPMultisampleState(&mMultisampling);
        pipelineInfo.setPColorBlendState(&colorBlending);
        pipelineInfo.setPDepthStencilState(&mDepthStencil);
        pipelineInfo.setLayout(mPipelineLayout);

        vk::DynamicState state[] = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

        vk::PipelineDynamicStateCreateInfo dynamicInfo{};
        dynamicInfo.setDynamicStates(state);

        pipelineInfo.setPDynamicState(&dynamicInfo);

        vk::Pipeline newPipeline{};

        VK_CHECK(device.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline));

        return newPipeline;
    }

    void setShaders(vk::ShaderModule vertexShader, vk::ShaderModule fragmentShader)
    {
        mShaderStages.clear();

        vk::PipelineShaderStageCreateInfo vertInfo{};
        vertInfo.setStage(vk::ShaderStageFlagBits::eVertex);
        vertInfo.setModule(vertexShader);
        vertInfo.setPName("main");
        mShaderStages.push_back(vertInfo);

        vk::PipelineShaderStageCreateInfo fragInfo{};
        fragInfo.setStage(vk::ShaderStageFlagBits::eFragment);
        fragInfo.setModule(fragmentShader);
        fragInfo.setPName("main");
        mShaderStages.push_back(fragInfo);
    }

    void setInputTopology(vk::PrimitiveTopology topology)
    {
        mInputAssembly.setTopology(topology);
        mInputAssembly.setPrimitiveRestartEnable(vk::False);
    }

    void setPolygonMode(vk::PolygonMode mode)
    {
        mRasterizer.setPolygonMode(mode);
        mRasterizer.setLineWidth(1.0f);
    }

    void setCullMode(vk::CullModeFlags cullMode, vk::FrontFace frontFace)
    {
        mRasterizer.setCullMode(cullMode);
        mRasterizer.setFrontFace(frontFace);
    }

    void setMultisamplingNone()
    {
        mMultisampling.setSampleShadingEnable(vk::False);
        mMultisampling.setRasterizationSamples(vk::SampleCountFlagBits::e1);
        mMultisampling.setMinSampleShading(1.0f);
        mMultisampling.setPSampleMask(nullptr);
        mMultisampling.setAlphaToCoverageEnable(vk::False);
        mMultisampling.setAlphaToOneEnable(vk::False);
    }

    void disableBlending()
    {
        mColorBlendAttachment.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        mColorBlendAttachment.setBlendEnable(vk::False);
    }

};


}
