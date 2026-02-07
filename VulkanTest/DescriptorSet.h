#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>

#include "Constants.h"
#include "DescriptorSetLayout.h"
#include "Devices.h"
#include "UniformBuffer.h"
#include "items/ItemInterface.h"
#include "StorageImageManager.h"

class DescriptorSet
{
public:
	DescriptorSet(DescriptorSetLayout& descriptorSetLayout, Devices& devices, UniformBuffer& uniformBuffer, 
		StorageImageManager& storageImageManager);
	// | GEOMETRY
	void createMeshDescriptorSets(ItemInterface& classReference);
	void createMandelbulbComputeDescriptorSets();
	void createMandelbulbGraphicsDescriptorSets();
	void createDescriptorPool();

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkDescriptorSet> mandelbulbComputeDescriptorSets;
	std::vector<VkDescriptorSet> mandelbulbGraphicsDescriptorSets;

private:
	Devices& m_Devices;
	DescriptorSetLayout& m_DescriptorSetLayout;
	UniformBuffer& m_UniformBuffer;
	StorageImageManager& m_StorageImageManager;

};

