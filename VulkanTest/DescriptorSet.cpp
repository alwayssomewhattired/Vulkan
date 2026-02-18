#include "DescriptorSet.h"

#include "DescriptorSetLayout.h"
#include "Devices.h"
#include "UniformBuffer.h"
#include "items/ItemInterface.h"
#include "StorageImageManager.h"
#include "Texture.h"


DescriptorSet::DescriptorSet(DescriptorSetLayout& descriptorSetLayout, Devices& devices, UniformBuffer& uniformBuffer, 
	StorageImageManager& storageImageManager, Texture& texture) : 
	m_DescriptorSetLayout(descriptorSetLayout), m_Devices(devices), m_UniformBuffer(uniformBuffer),
	m_StorageImageManager(storageImageManager), m_Texture(texture) {}


// | generic descriptor set creator
// - implement dynamic-uniform-buffer instead
void DescriptorSet::createMeshDescriptorSets(ItemInterface& classReference) {
	
	const auto& materials = classReference.gltfMaterials();

	const auto& textures = m_Texture.m_gpuTextures;

	auto& descriptorSets = classReference.descriptorSets();

	descriptorSets.resize(Constants::MAX_FRAMES_IN_FLIGHT * materials.size());

	std::vector<VkDescriptorSetLayout> layouts(descriptorSets.size(), m_DescriptorSetLayout.descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorSetLayout.descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSets.size());
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(m_Devices.device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate descriptor sets!");

	for (size_t frame = 0; frame < Constants::MAX_FRAMES_IN_FLIGHT; frame++) {
		for (size_t matIdx = 0; matIdx < materials.size(); ++matIdx) {

			size_t dsIndex = frame * 1 + matIdx;

			VkDescriptorSet dstSet = descriptorSets[dsIndex];

			const auto& material = materials[matIdx];
			const GPUTexture& GPUBaseColorTex = textures[material.baseColorTex];

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_UniformBuffer.uniformBuffers[frame];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(Camera::CameraUBO);

			VkDescriptorBufferInfo modelBufferInfo{};
			modelBufferInfo.buffer = m_UniformBuffer.modelUniformBuffers[frame];
			modelBufferInfo.offset = 0;
			modelBufferInfo.range = m_UniformBuffer.modelUBOSize;
			
			assert(material.baseColorTex < textures.size());
			VkDescriptorImageInfo baseColorInfo{};
			baseColorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			baseColorInfo.imageView = GPUBaseColorTex.view;
			baseColorInfo.sampler = GPUBaseColorTex.sampler;

			VkDescriptorImageInfo normalInfo{};
			normalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			normalInfo.imageView = textures[material.normalTex].view;
			normalInfo.sampler = textures[material.normalTex].sampler;

			std::array<VkWriteDescriptorSet, 4> descriptorWrites{};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = dstSet;
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = dstSet;
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &modelBufferInfo;

			descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[2].dstSet = dstSet;
			descriptorWrites[2].dstBinding = 2;
			descriptorWrites[2].dstArrayElement = 0;
			descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[2].descriptorCount = 1;
			descriptorWrites[2].pImageInfo = &baseColorInfo;

			descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[3].dstSet = dstSet;
			descriptorWrites[3].dstBinding = 3;
			descriptorWrites[3].dstArrayElement = 0;
			descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[3].descriptorCount = 1;
			descriptorWrites[3].pImageInfo = &normalInfo;
			vkUpdateDescriptorSets(m_Devices.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 
				0, nullptr);

		}
	}
}

void DescriptorSet::createMandelbulbComputeDescriptorSets() {

	mandelbulbComputeDescriptorSets.resize(Constants::MAX_FRAMES_IN_FLIGHT);

	std::vector<VkDescriptorSetLayout> layouts(Constants::MAX_FRAMES_IN_FLIGHT, 
		m_DescriptorSetLayout.mandelbulbComputeDescriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorSetLayout.computeDescriptorPool;            // must support STORAGE_IMAGE + UBO
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
		m_DescriptorSetLayout.mandelbulbGraphicsDescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorSetLayout.graphicsDescriptorPool;
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