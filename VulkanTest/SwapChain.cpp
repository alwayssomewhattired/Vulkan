#include "SwapChain.h"
#include "Devices.h"

#include <stdexcept>
#include <vulkan/vulkan_core.h>

SwapChain::SwapChain(Devices& devices, VkSurfaceKHR& surface) : 
	m_Devices(devices), m_SwapChainSupportDetails(devices.m_SwapChainSupportDetails), m_surface(surface){}

void SwapChain::createSwapChain() {
	Devices::SwapChainSupportDetails swapChainSupport = m_SwapChainSupportDetails;

	VkSurfaceFormatKHR surfaceFormat = m_Devices.chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = m_Devices.chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = m_Devices.chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
		imageCount = swapChainSupport.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	swapChainImageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	swapChainExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	//QueueFamilyIndices indices = findQueueFamilies(*m_physicalDevice);
	uint32_t queueFamilyIndices[] = { m_Devices.m_QueueFamilyIndices.graphicsFamily.value(),
		m_Devices.m_QueueFamilyIndices.presentFamily.value() };

	if (m_Devices.m_QueueFamilyIndices.graphicsFamily != m_Devices.m_QueueFamilyIndices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(m_Devices.device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		throw std::runtime_error("failed to create swapchain!");

	vkGetSwapchainImagesKHR(m_Devices.device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_Devices.device, swapChain, &imageCount, swapChainImages.data());
	swapChainImageCount = imageCount;
}

void SwapChain::cleanupSwapChain() {
	vkDestroyImage(*m_device, colorImage, nullptr);
	vkDestroyImageView(*m_device, colorImageView, nullptr);
	vkFreeMemory(*m_device, colorImageMemory, nullptr);
	vkDestroyImageView(*m_device, depthImageView, nullptr);
	vkDestroyImage(*m_device, depthImage, nullptr);
	vkFreeMemory(*m_device, depthImageMemory, nullptr);

	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(*m_device, framebuffer, nullptr);
	}

	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(*m_device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(*m_device, m_SwapChain->swapChain, nullptr);
}

void SwapChain::recreateSwapChain() {
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(*m_device);

	cleanupSwapChain();

	m_SwapChain->createSwapChain();
	createImageViews();
	createColorResources();
	createDepthResources();
	createFramebuffers();
	createSyncObjects();
}