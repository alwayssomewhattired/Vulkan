#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <functional>
#include <cstdint>

class HostToDevice
{
public:
	HostToDevice(const VkDevice& device, const VkPhysicalDevice& physicalDevice);
	void createUniformBuffer(const size_t& UBOSize);
	void createDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets, VkDescriptorSetLayout& descriptorSetLayout, 
		VkDescriptorPool& descriptorPool, std::vector<VkBuffer>& uniformBuffers, 
		const std::vector<VkBuffer>& modelUniformBuffers, VkImageView& textureImageView, VkSampler& textureSampler,
		size_t& UBOSize);

private:
	const VkDevice& m_device;
	const VkPhysicalDevice& m_physicalDevice;
};

