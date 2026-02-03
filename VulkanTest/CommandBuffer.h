#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "Devices.h"

class CommandBuffer
{
public:
	CommandBuffer(VkCommandPool& commandPool, Devices& devices);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

private:
	VkCommandPool& m_commandPool;
	Devices& m_devices;
};

