#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

namespace gegege::vulkan {

class DescriptorLayoutBuilder {

    std::vector<vk::DescriptorSetLayoutBinding> mBindings;

public:
    void add_binding(uint32_t binding, vk::DescriptorType type)
    {
        vk::DescriptorSetLayoutBinding newBind{};
        newBind.setBinding(binding);
        newBind.setDescriptorCount(1);
        newBind.setDescriptorType(type);

        mBindings.push_back(newBind);
    }

    void clear()
    {
        mBindings.clear();
    }

    vk::DescriptorSetLayout build(vk::Device device, vk::ShaderStageFlags shaderStages, void* pNext = nullptr, vk::DescriptorSetLayoutCreateFlags flags = {})
    {
        for (auto& b : mBindings)
        {
            b.stageFlags |= shaderStages;
        }

        vk::DescriptorSetLayoutCreateInfo info{};
        info.setPNext(pNext);

        info.setBindings(mBindings);
        info.setFlags(flags);

        vk::DescriptorSetLayout set{};
        device.createDescriptorSetLayout(&info, nullptr, &set);

        return set;
    }
};

class DescriptorAllocator {

public:
    struct PoolSizeRatio {
        vk::DescriptorType mType;
        float mRatio;
    };

private:
    vk::DescriptorPool mPool;

public:
    void initPool(vk::Device device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
    {
        std::vector<vk::DescriptorPoolSize> poolSize;
        for (PoolSizeRatio ratio : poolRatios)
        {
            vk::DescriptorPoolSize ps{};
            ps.setType(ratio.mType);
            ps.setDescriptorCount(uint32_t(ratio.mRatio * maxSets));
            poolSize.push_back(ps);
        }

        vk::DescriptorPoolCreateInfo poolInfo{};
        poolInfo.setMaxSets(maxSets);
        poolInfo.setPoolSizes(poolSize);

        device.createDescriptorPool(&poolInfo, nullptr, &mPool);
    }

    void clearDescriptors(vk::Device device)
    {
        device.resetDescriptorPool(mPool);
    }

    void destroyPool(vk::Device device)
    {
        device.destroyDescriptorPool(mPool, nullptr);
    }

    vk::DescriptorSet allocate(vk::Device device, vk::DescriptorSetLayout layout)
    {
        vk::DescriptorSetAllocateInfo allocInfo{};
        allocInfo.setDescriptorPool(mPool);
        allocInfo.setDescriptorSetCount(1);
        allocInfo.setPSetLayouts(&layout);

        vk::DescriptorSet ds{};
        device.allocateDescriptorSets(&allocInfo, &ds);

        return ds;
    }
};

} // namespace gegege::vulkan
