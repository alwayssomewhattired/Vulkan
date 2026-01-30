#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <stdexcept>

class DescriptorSetLayout
{
public:
	DescriptorSetLayout(VkDevice& device);

	void createMeshDescriptorSetLayout();

	VkDescriptorSetLayout descriptorSetLayout;

	void createMandelbulbComputeDescriptorSetLayout();
	VkDescriptorSetLayout mandelbulbComputeDescriptorSetLayout;

	void createMandelbulbGraphicsDescriptorSetLayout();
	VkDescriptorSetLayout mandelbulbGraphicsDescriptorSetLayout;

private:
	VkDevice& m_device;
};

