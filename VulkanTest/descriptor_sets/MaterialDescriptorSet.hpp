#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <iostream>

#include "../items/ItemInterface.h"
#include "DescriptorSetLayout.h"
#include "../Devices.h"

struct MaterialDescriptorSet {

	MaterialDescriptorSet(descriptorSetLayout& _descriptorSetLayout, Devices& devices) :
		m_descriptorSetLayout(_descriptorSetLayout), m_Devices(devices) {
	};

	VkDescriptorSet materialDescriptorSet;

	void createMaterialDescriptorSet(const uint32_t maxTextures) {

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_descriptorSetLayout.materialDescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_descriptorSetLayout.materialDescriptorSetLayout;

		VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{};
		countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
		countInfo.descriptorSetCount = 1;
		countInfo.pDescriptorCounts = &maxTextures;

		allocInfo.pNext = &countInfo;

		if (vkAllocateDescriptorSets(
			m_Devices.device,
			&allocInfo,
			&materialDescriptorSet) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
	}

	// | writes one texture (GPUMaterial) into slot
	// | index *** The 'slot' of the bindless array
	void updateMaterialDescriptorSet(uint32_t index, VkImageView imageView, VkSampler sampler)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = imageView;
		imageInfo.sampler = sampler;
		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = materialDescriptorSet;
		write.dstBinding = 0;
		write.dstArrayElement = index;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.descriptorCount = 1;
		write.pImageInfo = &imageInfo;
		vkUpdateDescriptorSets(m_Devices.device, 1, &write, 0, nullptr);
	}

private:
	descriptorSetLayout& m_descriptorSetLayout;
	Devices& m_Devices;
};