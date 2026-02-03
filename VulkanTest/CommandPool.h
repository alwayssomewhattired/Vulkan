#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "Devices.h"
#include <stdexcept>
class CommandPool
{
public:
	void createCommandPool(Devices& devices);
	VkCommandPool commandPool;
};

