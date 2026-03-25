#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "../Devices.h"
#include "../Constants.h"

#include <iostream>
#include <stdexcept>

class Devices;

class descriptorSetLayout
{
public:
	descriptorSetLayout(Devices& devices);
	void createGlobalDescriptorSetLayout();
	void createMeshdescriptorSetLayout();
	void createAnimationDescriptorSetLayout();
	void createMaterialDescriptorSetLayout();
	void createDescriptorPool(uint32_t materialCount, uint32_t numberModels);
	void createMaterialDescriptorPool();

	VkDescriptorSetLayout globalDescriptorSetLayout;
	VkDescriptorSetLayout meshDescriptorSetLayout;
	VkDescriptorSetLayout materialDescriptorSetLayout;
	VkDescriptorSetLayout animationDescriptorSetLayout;

	VkDescriptorPool descriptorPool;
	VkDescriptorPool materialDescriptorPool;
	VkDescriptorPool computeDescriptorPool;
	VkDescriptorPool graphicsDescriptorPool;

	void createMandelbulbComputedescriptorSetLayout();
	VkDescriptorSetLayout mandelbulbComputedescriptorSetLayout;
	void createComputeDescriptorPool(uint32_t computeSetCount);

	void createMandelbulbGraphicsdescriptorSetLayout();
	VkDescriptorSetLayout mandelbulbGraphicsdescriptorSetLayout;
	void createGraphicsDescriptorPool(uint32_t graphicsSetCount);

private:
	Devices& m_Devices;
};

