#pragma once

#include <vulkan/vulkan.h>
#include <utility>

class GPUTexture {

public:

    GPUTexture(VkDevice device);
    ~GPUTexture();

    GPUTexture(const GPUTexture&) = delete;
    GPUTexture& operator=(const GPUTexture&) = delete;

    GPUTexture(GPUTexture&& other) noexcept
        : image(other.image),
        memory(other.memory),
        view(other.view),
        sampler(other.sampler),
        //textureIndex(other.textureIndex),
        m_device(other.m_device)
    {
        other.image = VK_NULL_HANDLE;
        other.memory = VK_NULL_HANDLE;
        other.view = VK_NULL_HANDLE;
        other.sampler = VK_NULL_HANDLE;
        //other.textureIndex = 0;
    }

    GPUTexture& operator=(GPUTexture&& other) noexcept {
        if (this != &other) {

            image = other.image;
            memory = other.memory;
            view = other.view;
            sampler = other.sampler;
            //textureIndex = other.textureIndex;
            m_device = other.m_device;

            other.image = VK_NULL_HANDLE;
            other.memory = VK_NULL_HANDLE;
            other.view = VK_NULL_HANDLE;
            other.sampler = VK_NULL_HANDLE;
            //other.textureIndex = 0;
        }
        return *this;
    }

    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;

    // | index into bindless texture
    //uint32_t textureIndex = 0;

private:
    VkDevice m_device = VK_NULL_HANDLE;
};
