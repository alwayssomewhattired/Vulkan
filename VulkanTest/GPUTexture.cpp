#include "GPUTexture.h"

GPUTexture::GPUTexture(VkDevice device) : m_device(device) {}

GPUTexture::~GPUTexture() {
    if (sampler)
        vkDestroySampler(m_device, sampler, nullptr);
    if (view)
        vkDestroyImageView(m_device, view, nullptr);
    if (image)
        vkDestroyImage(m_device, image, nullptr);
    if (memory)
        vkFreeMemory(m_device, memory, nullptr);
}


