#include "DescriptorSetLayout.h"


#include <array>

descriptorSetLayout::descriptorSetLayout(Devices& devices) : m_Devices(devices){
}

void descriptorSetLayout::createGlobalDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding cameraBinding{};
	cameraBinding.binding = 0;
	cameraBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraBinding.descriptorCount = 1;
	cameraBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	cameraBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo globalLayoutInfo{};
	globalLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	globalLayoutInfo.bindingCount = 1;
	globalLayoutInfo.pBindings = &cameraBinding;

	if (vkCreateDescriptorSetLayout(m_Devices.device, &globalLayoutInfo, nullptr, &globalDescriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout!");

}

void descriptorSetLayout::createMeshdescriptorSetLayout() {

	VkDescriptorSetLayoutBinding baseColorFactorBinding{};
	baseColorFactorBinding.binding = 0;
	baseColorFactorBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	baseColorFactorBinding.descriptorCount = 1;
	baseColorFactorBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	baseColorFactorBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &baseColorFactorBinding;

	if (vkCreateDescriptorSetLayout(m_Devices.device, &layoutInfo, nullptr, &meshDescriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout!");
}

void descriptorSetLayout::createAnimationDescriptorSetLayout() {

	VkDescriptorSetLayoutBinding animationBinding{};
	animationBinding.binding = 0;
	animationBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	animationBinding.descriptorCount = 1;
	animationBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	animationBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &animationBinding;

	if (vkCreateDescriptorSetLayout(m_Devices.device, &layoutInfo, nullptr, &animationDescriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout!");
}

// | bindless
void descriptorSetLayout::createMaterialDescriptorSetLayout() {

	VkDescriptorSetLayoutBinding materialBinding{};
	materialBinding.binding = 0;
	materialBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	materialBinding.descriptorCount = m_Devices.maxTextures;
	materialBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	materialBinding.pImmutableSamplers = nullptr;

	VkDescriptorBindingFlags flags =
		VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | 
		VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
		VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

	VkDescriptorSetLayoutBindingFlagsCreateInfo extended{};
	extended.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
	extended.bindingCount = 1;
	extended.pBindingFlags = &flags;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &materialBinding;
	layoutInfo.pNext = &extended;
	layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

	if (vkCreateDescriptorSetLayout(m_Devices.device, &layoutInfo, nullptr, &materialDescriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create descriptor set layout!");
}

void descriptorSetLayout::createDescriptorPool(uint32_t materialCount, uint32_t numberModels) {

	uint32_t frames = Constants::MAX_FRAMES_IN_FLIGHT;

	uint32_t materialSetCount = frames * materialCount;
	uint32_t globalSetCount = frames;
	uint32_t animSetCount = frames * numberModels;

	std::array<VkDescriptorPoolSize, 1> poolSizes{};

	// camera UBO + material UBOs + anim UBOs
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount =
		//globalSetCount + animSetCount;
		globalSetCount + materialSetCount + animSetCount;

	// material textures
	//poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//poolSizes[1].descriptorCount =
	//	materialSetCount * 2;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();

	// total descriptor sets allocated from this pool
	poolInfo.maxSets =
		//globalSetCount + animSetCount;
		globalSetCount + materialSetCount + animSetCount;

	if (vkCreateDescriptorPool(
		m_Devices.device,
		&poolInfo,
		nullptr,
		&descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}

}

// | BINDLESS
void descriptorSetLayout::createMaterialDescriptorPool() {

	VkDescriptorPoolSize poolSize{};

	// material textures
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = m_Devices.maxTextures;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

	poolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(
		m_Devices.device,
		&poolInfo,
		nullptr,
		&materialDescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}

}

void descriptorSetLayout::createMandelbulbComputedescriptorSetLayout() {
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

	if (vkCreateDescriptorSetLayout(m_Devices.device, &layoutInfo, nullptr, &mandelbulbComputedescriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create compute descriptor set layout!");
	}
}

void descriptorSetLayout::createComputeDescriptorPool(uint32_t computeSetCount) {
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

void descriptorSetLayout::createMandelbulbGraphicsdescriptorSetLayout() {

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

	if (vkCreateDescriptorSetLayout(m_Devices.device, &layoutInfo, nullptr, &mandelbulbGraphicsdescriptorSetLayout) != VK_SUCCESS)
		throw std::runtime_error("failed to create mandelbulb descriptor set layout\n");
}

void descriptorSetLayout::createGraphicsDescriptorPool(uint32_t graphicsSetCount) {

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