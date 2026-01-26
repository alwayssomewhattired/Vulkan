#include "SilentHill3Game.h"
#include "../Constants.h"
#include <stdexcept>



void SilentHill3Game::createUniformBuffer() {

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

		if (vkCreateBuffer(device, &modelBufferInfo, nullptr, &modelUniformBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to create uniform buffer!");

		VkMemoryRequirements modelMemRequirements;
		vkGetBufferMemoryRequirements(device, modelUniformBuffers[i], &modelMemRequirements);

		VkMemoryAllocateInfo modelAllocInfo{};
		modelAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		modelAllocInfo.allocationSize = modelMemRequirements.size;
		modelAllocInfo.memoryTypeIndex = findMemoryType(modelMemRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(device, &modelAllocInfo, nullptr, &modelUniformBuffersMemory[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate uniform buffer memory");

		vkBindBufferMemory(device, modelUniformBuffers[i], modelUniformBuffersMemory[i], 0);

		vkMapMemory(device, modelUniformBuffersMemory[i], 0, modelBufferSize, 0, &modelUniformBuffersMapped[i]);
	}
}