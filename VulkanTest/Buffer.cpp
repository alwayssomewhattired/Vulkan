#include "Buffer.h"

#include "Vertex.hpp"
#include "Devices.h"
#include "CommandBuffer.h"
#include "Texture.h"

Buffer::Buffer(Devices& devices, CommandBuffer& commandBuffer) : m_Devices(devices), m_CommandBuffer(commandBuffer){}

void Buffer::createIndexAndVertexBuffer()
{
	VkDevice device = m_Devices.device;
	VkDeviceMemory vertexMemoryManager = VK_NULL_HANDLE;
	VkDeviceMemory indexMemoryManager = VK_NULL_HANDLE;

	VkDeviceSize vertexSize = sizeof(Vertex) * globalVertices.size();
	VkBuffer stagingVb;
	VkDeviceMemory stagingVm;

	// staging buffer
	createBuffer(
		vertexSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingVb,
		stagingVm
	);

	void* mapped;
	vkMapMemory(device, stagingVm, 0, vertexSize, 0, &mapped);
	memcpy(mapped, globalVertices.data(), static_cast<size_t>(vertexSize));
	vkUnmapMemory(device, stagingVm);

	auto& vertexBuffer = globalVertexBuffer;
	auto& indexBuffer = globalIndexBuffer;

	createBuffer(
		vertexSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer,
		vertexMemoryManager
	);

	// copy to gpu
	m_CommandBuffer.copyBuffer(stagingVb, vertexBuffer, vertexSize);

	// destroy staging buffer
	vkDestroyBuffer(device, stagingVb, nullptr);
	vkFreeMemory(device, stagingVm, nullptr);

	// globalIndices

	uint32_t indexCount = static_cast<uint32_t>(globalIndices.size());

	VkDeviceSize indexSize = sizeof(uint32_t) * globalIndices.size();

	VkBuffer stagingIb;
	VkDeviceMemory stagingIm;

	createBuffer(
		indexSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingIb,
		stagingIm
	);

	vkMapMemory(device, stagingIm, 0, indexSize, 0, &mapped);
	memcpy(mapped, globalIndices.data(), indexSize);
	vkUnmapMemory(device, stagingIm);

	createBuffer(
		indexSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer,
		indexMemoryManager
	);

	m_CommandBuffer.copyBuffer(stagingIb, indexBuffer, indexSize);

	vkDestroyBuffer(device, stagingIb, nullptr);
	vkFreeMemory(device, stagingIm, nullptr);
}

// | outdated and we don't use it
void Buffer::createIndexBuffer(const std::vector<Vertex>& triangleVertices, VkBuffer& indexBuffer, 
	VkDeviceMemory& indexBufferMemory) {


	// render triangle
	VkDeviceSize bufferSize = sizeof(triangleVertices[0]) * triangleVertices.size();
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(m_Devices.device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, triangleVertices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_Devices.device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer, indexBufferMemory);

	m_CommandBuffer.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(m_Devices.device, stagingBuffer, nullptr);
	vkFreeMemory(m_Devices.device, stagingBufferMemory, nullptr);

}

void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, 
	VkDeviceMemory& bufferMemory) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_Devices.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
		throw std::runtime_error("failed to create buffer!");

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_Devices.device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = m_Devices.findMemoryType(memRequirements.memoryTypeBits, properties, m_Devices.physicalDevice);

	if (vkAllocateMemory(m_Devices.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate buffer memory!");

	vkBindBufferMemory(m_Devices.device, buffer, bufferMemory, 0);
}


void Buffer::createVertexBuffer(const std::vector<Vertex>& triangleVertices, 
	std::vector<VkBuffer>& triangleVertexBufferManager,
	std::vector<VkDeviceMemory>& triangleVertexBufferMemoryManager) {

	int i = 0;

	VkBuffer triangleVertexBuffer;
	VkDeviceMemory triangleVertexBufferMemory;

	// TRIANGLE
	//
	//
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkDeviceSize triangleBufferSize = sizeof(triangleVertices[0]) * triangleVertices.size();
	void* data;

	createBuffer(triangleBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	vkMapMemory(m_Devices.device, stagingBufferMemory, 0, triangleBufferSize, 0, &data);
	memcpy(data, triangleVertices.data(), (size_t)triangleBufferSize);
	vkUnmapMemory(m_Devices.device, stagingBufferMemory);

	createBuffer(triangleBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		triangleVertexBuffer, triangleVertexBufferMemory);

	m_CommandBuffer.copyBuffer(stagingBuffer, triangleVertexBuffer, triangleBufferSize);

	vkDestroyBuffer(m_Devices.device, stagingBuffer, nullptr);
	vkFreeMemory(m_Devices.device, stagingBufferMemory, nullptr);

	triangleVertexBufferManager.push_back(std::move(triangleVertexBuffer));
	triangleVertexBufferMemoryManager.push_back(std::move(triangleVertexBufferMemory));

}

// | material ssbo
void Buffer::createSSBO(std::vector<Texture::ItemMaterial>& globalItemMaterials)
{

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkDeviceSize bufferSize = sizeof(Texture::ItemMaterial) * globalItemMaterials.size();
	void* data;

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	vkMapMemory(m_Devices.device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, globalItemMaterials.data(), (size_t)bufferSize);
	vkUnmapMemory(m_Devices.device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		materialSSBO, materialSSBOMemory);

	m_CommandBuffer.copyBuffer(stagingBuffer, materialSSBO, bufferSize);

	vkDestroyBuffer(m_Devices.device, stagingBuffer, nullptr);
	vkFreeMemory(m_Devices.device, stagingBufferMemory, nullptr);

}

void Buffer::createBonesSSBO(std::vector<glm::mat4>& globalFinalBoneMatrices)
{

	VkDeviceSize bufferSize = sizeof(glm::mat4) * globalFinalBoneMatrices.size();

	createBuffer(
		bufferSize, 
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		animationSSBO, 
		animationSSBOMemory);

	vkMapMemory(m_Devices.device, animationSSBOMemory, 0, bufferSize, 0, &mappedBonesPtr);

}

void Buffer::updateBonesSSBO(const std::vector<glm::mat4>& globalFinalBoneMatrices) {
	VkDeviceSize bufferSize = sizeof(glm::mat4) * globalFinalBoneMatrices.size();

	memcpy(mappedBonesPtr, globalFinalBoneMatrices.data(), bufferSize);
}