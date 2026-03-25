#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <array>
#include <iostream>
#include <cstdint>
#include <vector>

#include "descriptor_sets/MaterialDescriptorSet.hpp"

class SwapChain;

	class Devices;
	class CommandPool;
	class GraphicsPipeline;
	class RenderTarget;
	class ItemInterface;
	class Home;
	class SilentHill3Game;
	class DescriptorSet;
	class UniformBuffer;


class CommandBuffer
{
public:
	CommandBuffer(VkCommandPool& commandPool, Devices& devices, SwapChain& swapChain);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	void createCommandBuffers(CommandPool& commandPool);

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VkRenderPass& renderPass,
		GraphicsPipeline& graphicsPipeline, std::vector<ItemInterface*>& items, DescriptorSet& descriptorSet, 
		const uint32_t currentFrame,
		VkImage& storageImage, ItemInterface& triangleClass, UniformBuffer& uniformBuffer, 
		MaterialDescriptorSet& materialDescriptorSet);

	std::vector<VkCommandBuffer> commandBuffers;

private:
	VkCommandPool& m_commandPool;
	Devices& m_devices;
	SwapChain& m_SwapChain;
};

