#include "DescriptorSet.h"

#include "DescriptorSetLayout.h"
#include "../Devices.h"
#include "../UniformBuffer.h"
#include "../items/ItemInterface.h"
#include "../StorageImageManager.h"
#include "../Texture.h"


DescriptorSet::DescriptorSet(descriptorSetLayout& descriptorSetLayout, Devices& devices, UniformBuffer& uniformBuffer, 
	StorageImageManager& storageImageManager, Texture& texture) : 
	m_descriptorSetLayout(descriptorSetLayout), m_Devices(devices), m_UniformBuffer(uniformBuffer),
	m_StorageImageManager(storageImageManager), m_Texture(texture) {}

void DescriptorSet::createGlobalDescriptorSets() {

	auto& descriptorSets = globalDescriptorSets;
	
	descriptorSets.resize(Constants::MAX_FRAMES_IN_FLIGHT);

	std::vector<VkDescriptorSetLayout> layouts(
		descriptorSets.size(),
		m_descriptorSetLayout.globalDescriptorSetLayout
	);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorSetLayout.descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSets.size());
	allocInfo.pSetLayouts = layouts.data();
	if (vkAllocateDescriptorSets(
		m_Devices.device,
		&allocInfo,
		descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t frame = 0; frame < Constants::MAX_FRAMES_IN_FLIGHT; frame++) {

		VkDescriptorSet dstSet = descriptorSets[frame];

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_UniformBuffer.uniformBuffers[frame];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(Camera::CameraUBO);

		VkWriteDescriptorSet descriptorWrites{};

		descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites.dstSet = dstSet;
		descriptorWrites.dstBinding = 0;
		descriptorWrites.dstArrayElement = 0;
		descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites.descriptorCount = 1;
		descriptorWrites.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(m_Devices.device, 1, &descriptorWrites, 0, nullptr);

	}
};

// | material descriptor set creator
void DescriptorSet::createMeshMaterialDescriptorSet(VkBuffer& materialSSBO) {

	const auto& textures = m_Texture.globalItemMaterials;
	VkDescriptorSetLayout& meshMaterialDescriptorSetLayout = m_descriptorSetLayout.meshMaterialDescriptorSetLayout;

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorSetLayout.descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &meshMaterialDescriptorSetLayout;
	if (vkAllocateDescriptorSets(
		m_Devices.device,
		&allocInfo,
		&meshMaterialDescriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	VkDescriptorBufferInfo materialBufferInfo{};
	materialBufferInfo.buffer = materialSSBO;
	materialBufferInfo.offset = 0;
	materialBufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet descriptorWrite{};

	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = meshMaterialDescriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &materialBufferInfo;

	vkUpdateDescriptorSets(m_Devices.device, 1, &descriptorWrite, 0, nullptr);

}

void DescriptorSet::createAnimationDescriptorSets(VkBuffer& animationSSBO) {


	std::vector<VkDescriptorSetLayout> layouts(
		1,
		m_descriptorSetLayout.animationDescriptorSetLayout
	);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorSetLayout.descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts.data();
	if (vkAllocateDescriptorSets(
		m_Devices.device,
		&allocInfo,
		&animationDescriptorSet) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	VkDescriptorBufferInfo animationBufferInfo{};
	animationBufferInfo.buffer = animationSSBO;;
	animationBufferInfo.offset = 0;
	animationBufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet descriptorWrites{};

	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = animationDescriptorSet;
	descriptorWrites.dstBinding = 0;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrites.descriptorCount = 1;
	descriptorWrites.pBufferInfo = &animationBufferInfo;

	vkUpdateDescriptorSets(m_Devices.device, 1, &descriptorWrites,
		0, nullptr);

}


void DescriptorSet::createMandelbulbComputeDescriptorSets() {

	mandelbulbComputeDescriptorSets.resize(Constants::MAX_FRAMES_IN_FLIGHT);

	std::vector<VkDescriptorSetLayout> layouts(Constants::MAX_FRAMES_IN_FLIGHT, 
		m_descriptorSetLayout.mandelbulbComputedescriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorSetLayout.computeDescriptorPool;            // must support STORAGE_IMAGE + UBO
	allocInfo.descriptorSetCount = Constants::MAX_FRAMES_IN_FLIGHT;
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(m_Devices.device, &allocInfo, mandelbulbComputeDescriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate compute descriptor sets!");
	}

	for (size_t i = 0; i < Constants::MAX_FRAMES_IN_FLIGHT; i++) {

		// UBO
		VkDescriptorBufferInfo uboInfo{};
		uboInfo.buffer = m_UniformBuffer.mandelbulbUniformBuffers[i];
		uboInfo.offset = 0;
		uboInfo.range = sizeof(UniformBuffer::MandelbulbUBO);

		// Storage image
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = m_StorageImageManager.m_GPUStorageImage.view(); // created with STORAGE_IMAGE usage
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // for storage image writes

		std::array<VkWriteDescriptorSet, 2> writes{};

		// Binding 0: UBO
		writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[0].dstSet = mandelbulbComputeDescriptorSets[i];
		writes[0].dstBinding = 0;
		writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writes[0].descriptorCount = 1;
		writes[0].pBufferInfo = &uboInfo;

		// Binding 1: storage image
		writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[1].dstSet = mandelbulbComputeDescriptorSets[i];
		writes[1].dstBinding = 1;
		writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		writes[1].descriptorCount = 1;
		writes[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_Devices.device, writes.size(), writes.data(), 0, nullptr);
	}
}

void DescriptorSet::createMandelbulbGraphicsDescriptorSets() {

	mandelbulbGraphicsDescriptorSets.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	std::vector<VkDescriptorSetLayout> layouts(Constants::MAX_FRAMES_IN_FLIGHT, 
		m_descriptorSetLayout.mandelbulbGraphicsdescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorSetLayout.graphicsDescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(Constants::MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(m_Devices.device, &allocInfo, mandelbulbGraphicsDescriptorSets.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate mandelbulb descriptor sets!");

	m_StorageImageManager.createMandelbulbSampler();

	for (size_t i = 0; i < Constants::MAX_FRAMES_IN_FLIGHT; i++) {

		// binding 0: sample image (output of computer shader)
		VkDescriptorImageInfo imgInfo{};
		imgInfo.sampler = m_StorageImageManager.mandelbulbSampler;
		imgInfo.imageView = m_StorageImageManager.m_GPUStorageImage.view();
		imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// binding 1: UBO
		VkDescriptorBufferInfo mandelbulbBufferInfo{};
		mandelbulbBufferInfo.buffer = m_UniformBuffer.mandelbulbUniformBuffers[i];
		mandelbulbBufferInfo.offset = 0;
		mandelbulbBufferInfo.range = sizeof(UniformBuffer::MandelbulbUBO);

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		// sampler
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = mandelbulbGraphicsDescriptorSets[i];
		descriptorWrites[0].dstBinding = 0; // matches mandelbulb shader binding
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pImageInfo = &imgInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = mandelbulbGraphicsDescriptorSets[i];
		descriptorWrites[1].dstBinding = 1; // matches mandelbulb shader binding
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &mandelbulbBufferInfo;

		vkUpdateDescriptorSets(m_Devices.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}