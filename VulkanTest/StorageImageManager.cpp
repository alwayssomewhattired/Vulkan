#include "StorageImageManager.h"

StorageImageManager::StorageImageManager(Image& image, SwapChain& swapChain, Devices& devices) : 
	m_Image(image), m_SwapChain(swapChain), m_Devices(devices) {}

void StorageImageManager::createStorageImageResources() {
	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

	auto& storageImage = m_GPUStorageImage.image();
	auto& storageImageMemory = m_GPUStorageImage.memory();
	auto& storageImageView = m_GPUStorageImage.view();

	m_Image.createImage(m_SwapChain.swapChainExtent.width, m_SwapChain.swapChainExtent.height, 1, VK_SAMPLE_COUNT_1_BIT, format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		storageImage, storageImageMemory
	);
	m_Image.createImageView(storageImage, format, VK_IMAGE_ASPECT_COLOR_BIT, 1, m_GPUStorageImage);
}


// - could possibly be in wrong class...
void StorageImageManager::createMandelbulbSampler() {
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1.0f;

	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	samplerInfo.mipLodBias = 0.0f;

	if (vkCreateSampler(m_Devices.device, &samplerInfo, nullptr, &mandelbulbSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create sampler for mandelbulb");
	}

}