#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <array>
#include <iostream>
#include <cstdint>
#include <vector>

//#include "SwapChain.h"
//#include "Devices.h"
//#include "CommandPool.h"
//#include "GraphicsPipeline.h"
//#include "RenderTarget.h"
//#include "items/ItemInterface.h"
//#include "items/Home.h"
//#include "items/SilentHill3Game.h"
//#include "DescriptorSet.h"

class SwapChain;

	class Devices;
	class CommandPool;
	class GraphicsPipeline;
	class RenderTarget;
	class ItemInterface;
	class Home;
	class SilentHill3Game;
	class DescriptorSet;


class CommandBuffer
{
public:
	CommandBuffer(VkCommandPool& commandPool, Devices& devices, SwapChain& swapChain);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	void createCommandBuffers(CommandPool& commandPool);

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VkRenderPass& renderPass,
		GraphicsPipeline& graphicsPipeline, std::vector<ItemInterface*>& item, DescriptorSet& descriptorSet, 
		const uint32_t& currentFrame,
		VkImage& storageImage, VkBuffer& triangleVertexBuffer);

	std::vector<VkCommandBuffer> commandBuffers;

private:
	VkCommandPool& m_commandPool;
	Devices& m_devices;
	SwapChain& m_SwapChain;
};

