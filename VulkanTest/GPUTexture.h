#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <utility>

// | non-copyabe
class GPUTexture {
	
public:

	GPUTexture(VkDevice device);

	VkImage image = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkImageView view = VK_NULL_HANDLE;
	VkSampler sampler = VK_NULL_HANDLE;

	// | This code block is for making our struct non-copyable

	~GPUTexture();

	// | deletes copy constructor
	GPUTexture(const GPUTexture&) = delete;

	// | deletes copy assignment operator
	GPUTexture& operator=(const GPUTexture&) = delete;

	// | move constructor
	GPUTexture(GPUTexture&& other) noexcept {
		*this = std::move(other);
	}

	// | move assignment operator
	GPUTexture& operator=(GPUTexture&& other) noexcept {

		// | transfers handles
		image = other.image;
		memory = other.memory;
		view = other.view;
		sampler = other.sampler;

		// | nulls out the source
		other.image = VK_NULL_HANDLE;
		other.memory = VK_NULL_HANDLE;
		other.view = VK_NULL_HANDLE;
		other.sampler = VK_NULL_HANDLE;

	}

private:
	VkDevice m_device;
};