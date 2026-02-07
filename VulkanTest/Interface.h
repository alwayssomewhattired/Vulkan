#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

struct ImageBundle {
	virtual VkImage& image() = 0;
	virtual VkDeviceMemory& memory() = 0;
	virtual VkImageView& view() = 0;

	virtual ~ImageBundle() = default;
};