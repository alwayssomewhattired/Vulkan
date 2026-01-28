#include "HostToDevice.h"
#include "Constants.h"
#include "Devices.h"
#include "Camera.h"
#include <stdexcept>


HostToDevice::HostToDevice(const VkDevice& device, const VkPhysicalDevice& physicalDevice)
	: m_device(device), m_physicalDevice(physicalDevice){ }

void HostToDevice::createUniformBuffer(const size_t& UBOSize)
{

	std::vector<VkBuffer> modelUniformBuffers;
	std::vector<VkDeviceMemory> modelUniformBuffersMemory;
	std::vector<void*> modelUniformBuffersMapped;

	VkDeviceSize modelBufferSize = UBOSize;
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
		modelAllocInfo.memoryTypeIndex = Devices::findMemoryType(modelMemRequirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_physicalDevice);

		if (vkAllocateMemory(m_device, &modelAllocInfo, nullptr, &modelUniformBuffersMemory[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate uniform buffer memory");

		vkBindBufferMemory(m_device, modelUniformBuffers[i], modelUniformBuffersMemory[i], 0);

		vkMapMemory(m_device, modelUniformBuffersMemory[i], 0, modelBufferSize, 0, &modelUniformBuffersMapped[i]);
	}
}

void HostToDevice::createDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets, VkDescriptorSetLayout& descriptorSetLayout, 
	VkDescriptorPool& descriptorPool, std::vector<VkBuffer>& uniformBuffers, const std::vector<VkBuffer>& modelUniformBuffers,
	VkImageView& textureImageView, VkSampler& textureSampler, size_t& UBOSize) {

	descriptorSets.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	std::vector<VkDescriptorSetLayout> layouts(Constants::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(m_device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate descriptor sets!");

	for (size_t i = 0; i < Constants::MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(Camera::CameraUBO);

		VkDescriptorBufferInfo modelBufferInfo{};
		modelBufferInfo.buffer = modelUniformBuffers[i];
		modelBufferInfo.offset = 0;
		modelBufferInfo.range = UBOSize;

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &modelBufferInfo;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = descriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	}
}
