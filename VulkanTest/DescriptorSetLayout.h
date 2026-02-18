#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Devices.h"

#include <stdexcept>

class Devices;

class DescriptorSetLayout
{
public:
	DescriptorSetLayout(Devices& devices);
	void createMeshDescriptorSetLayout();
	void createDescriptorPool(uint32_t materialCount);

	VkDescriptorSetLayout descriptorSetLayout;

	VkDescriptorPool descriptorPool;
	VkDescriptorPool computeDescriptorPool;
	VkDescriptorPool graphicsDescriptorPool;

	void createMandelbulbComputeDescriptorSetLayout();
	VkDescriptorSetLayout mandelbulbComputeDescriptorSetLayout;
	void createComputeDescriptorPool(uint32_t computeSetCount);

	void createMandelbulbGraphicsDescriptorSetLayout();
	VkDescriptorSetLayout mandelbulbGraphicsDescriptorSetLayout;
	void createGraphicsDescriptorPool(uint32_t graphicsSetCount);

private:
	Devices& m_Devices;;
};

