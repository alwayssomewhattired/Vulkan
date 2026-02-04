#include "DescriptorSetLayout.h"
#include <array>

DescriptorSetLayout::DescriptorSetLayout(VkDevice& device) : m_device(device){}

void DescriptorSetLayout::createMeshDescriptorSetLayout() {


	VkDescriptorSetLayoutBinding cameraBinding{};
	cameraBinding.binding = 0;
	cameraBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraBinding.descriptorCount = 1;
	cameraBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	cameraBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding modelBinding{};
	modelBinding.binding = 1;
	modelBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	modelBinding.descriptorCount = 1;
	modelBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	modelBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding samplerBinding{};
	samplerBinding.binding = 2;
	samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBinding.descriptorCount = 1;
	samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding normalBinding{};
	normalBinding.binding = 3;
	normalBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalBinding.descriptorCount = 1;
	normalBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	normalBinding.pImmutableSamplers = nullptr;

	std::array<VkDescriptorSetLayoutBinding, 4> bindings = { cameraBinding, modelBinding, samplerBinding, normalBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout!");
}

void DescriptorSetLayout::createMandelbulbComputeDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding uboBinding{};
	uboBinding.binding = 0;
	uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboBinding.descriptorCount = 1;
	uboBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	uboBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding storageImageBinding{};
	storageImageBinding.binding = 1;
	storageImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	storageImageBinding.descriptorCount = 1;
	storageImageBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	storageImageBinding.pImmutableSamplers = nullptr;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
		uboBinding,
		storageImageBinding
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &mandelbulbComputeDescriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create compute descriptor set layout!");
	}
}

void DescriptorSetLayout::createMandelbulbGraphicsDescriptorSetLayout() {

	VkDescriptorSetLayoutBinding samplerBinding{};
	samplerBinding.binding = 0;
	samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBinding.descriptorCount = 1;
	samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding mandelbulbUBOBinding{};
	mandelbulbUBOBinding.binding = 1;
	mandelbulbUBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mandelbulbUBOBinding.descriptorCount = 1;
	mandelbulbUBOBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	mandelbulbUBOBinding.pImmutableSamplers = nullptr;


	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { samplerBinding, mandelbulbUBOBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &mandelbulbGraphicsDescriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create mandelbulb descriptor set layout\n");
}