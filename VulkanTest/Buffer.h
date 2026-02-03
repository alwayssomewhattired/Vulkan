#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <vector>
#include <stdexcept>

#include "Vertex.h"
#include "Devices.h"
#include "CommandBuffer.h"

// | a class of some buffer stuff
class Buffer
{
public:
	Buffer(Devices& devices, CommandBuffer& commandBuffer);

	void createIndexBuffer(const std::vector<Vertex>& triangleVertices, VkBuffer& indexBuffer,
		VkDeviceMemory& indexBufferMemory);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
		VkDeviceMemory& bufferMemory);
	void createVertexBuffer(const std::vector<Vertex>& triangleVertices, VkBuffer& triangleVertexBuffer,
		VkDeviceMemory& triangleVertexBufferMemory);

private:
	Devices& m_Devices;
	CommandBuffer& m_CommandBuffer;
};

