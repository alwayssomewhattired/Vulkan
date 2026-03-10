#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Devices.h"

#include <stdexcept>

class Devices;

class descriptorSetLayout
{
public:
	descriptorSetLayout(Devices& devices);
	void createGlobalDescriptorSetLayout();
	void createMeshdescriptorSetLayout();
	void createDescriptorPool(uint32_t materialCount);

	VkDescriptorSetLayout globalDescriptorSetLayout;
	VkDescriptorSetLayout materialDescriptorSetLayout;

	VkDescriptorPool descriptorPool;
	VkDescriptorPool computeDescriptorPool;
	VkDescriptorPool graphicsDescriptorPool;

	void createMandelbulbComputedescriptorSetLayout();
	VkDescriptorSetLayout mandelbulbComputedescriptorSetLayout;
	void createComputeDescriptorPool(uint32_t computeSetCount);

	void createMandelbulbGraphicsdescriptorSetLayout();
	VkDescriptorSetLayout mandelbulbGraphicsdescriptorSetLayout;
	void createGraphicsDescriptorPool(uint32_t graphicsSetCount);

private:
	Devices& m_Devices;;
};

