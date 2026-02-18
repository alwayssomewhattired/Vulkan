#include "DescriptorSetLayout.h"
#include "Constants.h"
#include <array>

DescriptorSetLayout::DescriptorSetLayout(Devices& devices) : m_Devices(devices){}

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

	if (vkCreateDescriptorSetLayout(m_Devices.device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout!");
}

void DescriptorSetLayout::createDescriptorPool(uint32_t materialCount) {

	uint32_t meshSetCount =
		Constants::MAX_FRAMES_IN_FLIGHT * materialCount;

	std::array<VkDescriptorPoolSize, 3> poolSizes{};

	// Camera UBO (binding 0)
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = meshSetCount;

	// Model UBO (binding 1, dynamic)
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = meshSetCount;

	// BaseColor + Normal samplers (bindings 2 & 3)
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = meshSetCount * 2;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = meshSetCount;

	if (vkCreateDescriptorPool(
		m_Devices.device,
		&poolInfo,
		nullptr,
		&descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
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

	if (vkCreateDescriptorSetLayout(m_Devices.device, &layoutInfo, nullptr, &mandelbulbComputeDescriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create compute descriptor set layout!");
	}
}

void DescriptorSetLayout::createComputeDescriptorPool(uint32_t computeSetCount) {
	std::array<VkDescriptorPoolSize, 2> poolSizes{};

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = computeSetCount;

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	poolSizes[1].descriptorCount = computeSetCount;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = computeSetCount;

	if (vkCreateDescriptorPool(
		m_Devices.device,
		&poolInfo,
		nullptr,
		&computeDescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
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

	if (vkCreateDescriptorSetLayout(m_Devices.device, &layoutInfo, nullptr, &mandelbulbGraphicsDescriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create mandelbulb descriptor set layout\n");
}

void DescriptorSetLayout::createGraphicsDescriptorPool(uint32_t graphicsSetCount) {

	std::array<VkDescriptorPoolSize, 2> poolSizes{};

	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = graphicsSetCount;

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = graphicsSetCount;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = graphicsSetCount;

	if (vkCreateDescriptorPool(
		m_Devices.device,
		&poolInfo,
		nullptr,
		&graphicsDescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}