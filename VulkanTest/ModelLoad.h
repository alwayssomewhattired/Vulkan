#pragma once

#include "tiny_gltf.h"

#include <iostream>
#include <stdexcept>
#include <functional>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <vector>

class ModelLoad
{
public:

	ModelLoad(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue,
		std::function<void(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&)> createBufferFn,
		std::function<void(VkBuffer, VkBuffer, VkDeviceSize)> copyBuffer
		);

	void loadModel(const std::string& path);

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexMemory;

	VkBuffer indexBuffer;
	VkDeviceMemory indexMemory;

	uint32_t indexCount;


private:
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;

	std::function<void(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&)> createBufferFn;
	std::function<void(VkBuffer, VkBuffer, VkDeviceSize)> copyBufferFn;

	std::vector<float> vertices;
	std::vector<uint32_t> indices;
};