#pragma once

#include <tiny_gltf.h>
#include "MathConfig.hpp"

#include <iostream>
#include <stdexcept>
#include <functional>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <vector>

#include "items/ItemInterface.h"
#include "Vertex.h"

// | model loader AND safe-room model class
// - yeah, not good. decouple safe-room model from model loading 
class ModelLoad
{
public:

	ModelLoad(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue,
		std::function<void(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&)> createBufferFn,
		std::function<void(VkBuffer, VkBuffer, VkDeviceSize)> copyBuffer
		);

	// | loads model and fills in class
	void loadModel(const std::string& path, ItemInterface& classReference);
	

private:
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;

	std::function<void(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&)> createBufferFn;
	std::function<void(VkBuffer, VkBuffer, VkDeviceSize)> copyBufferFn;

	// | staging mesh vertices
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};