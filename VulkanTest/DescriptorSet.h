#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>

#include "Constants.h"

class DescriptorSetLayout;
class Devices;
class UniformBuffer;
class StorageImageManager;
class ItemInterface;
class Texture;

class DescriptorSet
{
public:
	DescriptorSet(DescriptorSetLayout& descriptorSetLayout, Devices& devices, UniformBuffer& uniformBuffer, 
		StorageImageManager& storageImageManager, Texture& texture);
	// | GEOMETRY
	void createMeshDescriptorSets(ItemInterface& classReference);
	void createMandelbulbComputeDescriptorSets();
	void createMandelbulbGraphicsDescriptorSets();
	//void createDescriptorPool(uint32_t materialCount);

	/*VkDescriptorPool descriptorPool;*/

	std::vector<VkDescriptorSet> mandelbulbComputeDescriptorSets;
	std::vector<VkDescriptorSet> mandelbulbGraphicsDescriptorSets;

private:
	Devices& m_Devices;
	DescriptorSetLayout& m_DescriptorSetLayout;
	UniformBuffer& m_UniformBuffer;
	StorageImageManager& m_StorageImageManager;
	Texture& m_Texture;
};

