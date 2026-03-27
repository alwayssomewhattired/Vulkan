#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>

#include "../Constants.h"

class descriptorSetLayout;
class Devices;
class UniformBuffer;
class StorageImageManager;
class ItemInterface;
class Texture;

class DescriptorSet
{
public:
	DescriptorSet(descriptorSetLayout& descriptorSetLayout, Devices& devices, UniformBuffer& uniformBuffer, 
		StorageImageManager& storageImageManager, Texture& texture);

	// | GEOMETRY
	void createGlobalDescriptorSets();
	void createMeshMaterialDescriptorSet(VkBuffer& materialSSBO);
	void createAnimationDescriptorSets(ItemInterface& classReference);
	void createMandelbulbComputeDescriptorSets();
	void createMandelbulbGraphicsDescriptorSets();

	std::vector<VkDescriptorSet> globalDescriptorSets;
	VkDescriptorSet meshMaterialDescriptorSet;
	std::vector<VkDescriptorSet> mandelbulbComputeDescriptorSets;
	std::vector<VkDescriptorSet> mandelbulbGraphicsDescriptorSets;

private:
	Devices& m_Devices;
	descriptorSetLayout& m_descriptorSetLayout;
	UniformBuffer& m_UniformBuffer;
	StorageImageManager& m_StorageImageManager;
	Texture& m_Texture;
};

