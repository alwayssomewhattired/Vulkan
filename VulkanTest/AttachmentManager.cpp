#include "AttachmentManager.h"

AttachmentManager::AttachmentManager(SwapChain& swapChain, VkSampleCountFlagBits& msaaSamples, Image& image, Devices& devices) : 
	m_SwapChain(swapChain), m_msaaSamples(msaaSamples), m_Image(image), m_Devices(devices) {}

void AttachmentManager::createColorResources() {
	VkFormat colorFormat = m_SwapChain.swapChainImageFormat;

	auto& colorImage = m_GPUColor.image();
	auto& colorImageMemory = m_GPUColor.memory();
	auto& colorImageView = m_GPUColor.view();

	m_Image.createImage(m_SwapChain.swapChainExtent.width, m_SwapChain.swapChainExtent.height, 1, m_msaaSamples, colorFormat, 
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		colorImage, colorImageMemory);

	colorImageView = m_Image.createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void AttachmentManager::createDepthResources() {
	VkFormat depthFormat = findDepthFormat();

	auto& depthImage = m_GPUDepth.image();
	auto& depthImageMemory = m_GPUDepth.memory();
	auto& depthImageView = m_GPUDepth.view();

	m_Image.createImage(m_SwapChain.swapChainExtent.width, m_SwapChain.swapChainExtent.height, 1, m_msaaSamples, depthFormat, 
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);

	depthImageView = m_Image.createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

VkFormat AttachmentManager::findDepthFormat() {
	return m_Devices.findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool AttachmentManager::hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}