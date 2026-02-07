#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <array>

#include "SwapChain.h"
#include "Devices.h"
#include "CommandPool.h"
#include "GraphicsPipeline.h"

class CommandBuffer
{
public:
	CommandBuffer(VkCommandPool& commandPool, Devices& devices, SwapChain& swapChain);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	void createCommandBuffers(CommandPool& commandPool);

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VkRenderPass& renderPass,
		GraphicsPipeline& graphicsPipeline);

	std::vector<VkCommandBuffer> commandBuffers;

private:
	VkCommandPool& m_commandPool;
	Devices& m_devices;
	SwapChain& m_SwapChain;
};

