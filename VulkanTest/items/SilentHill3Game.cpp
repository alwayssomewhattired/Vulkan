#include "SilentHill3Game.h"
#include "../Constants.h"
#include <stdexcept>
#include <functional>
#include <cstdint>

SilentHill3Game::SilentHill3Game(VkDevice& device) : m_device(device) {}

void SilentHill3Game::createUniformBuffer(
	std::function<uint32_t(uint32_t typeFilter, VkMemoryPropertyFlags properties)>& findMemoryType) 
{

	std::vector<VkBuffer> modelUniformBuffers;
	std::vector<VkDeviceMemory> modelUniformBuffersMemory;
	std::vector<void*> modelUniformBuffersMapped;

	VkDeviceSize modelBufferSize = sizeof(ModelUBO);
	modelUniformBuffers.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	modelUniformBuffersMemory.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	modelUniformBuffersMapped.resize(Constants::MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < Constants::MAX_FRAMES_IN_FLIGHT; i++) {

		VkBufferCreateInfo modelBufferInfo{};
		modelBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		modelBufferInfo.size = modelBufferSize;
		modelBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		modelBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(m_device, &modelBufferInfo, nullptr, &modelUniformBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to create uniform buffer!");

		VkMemoryRequirements modelMemRequirements;
		vkGetBufferMemoryRequirements(m_device, modelUniformBuffers[i], &modelMemRequirements);

		VkMemoryAllocateInfo modelAllocInfo{};
		modelAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		modelAllocInfo.allocationSize = modelMemRequirements.size;
		modelAllocInfo.memoryTypeIndex = findMemoryType(modelMemRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(m_device, &modelAllocInfo, nullptr, &modelUniformBuffersMemory[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate uniform buffer memory");

		vkBindBufferMemory(m_device, modelUniformBuffers[i], modelUniformBuffersMemory[i], 0);

		vkMapMemory(m_device, modelUniformBuffersMemory[i], 0, modelBufferSize, 0, &modelUniformBuffersMapped[i]);
	}
}