#include "SwapChain.h"
#include "Devices.h"
#include "AttachmentManager.h"
#include "Image.h"
#include "RenderPass.h"

#include <stdexcept>
#include <vulkan/vulkan_core.h>

SwapChain::SwapChain(Devices& devices, VkSurfaceKHR surface) : 
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
	//swapChainImageCount = imageCount;
}

void SwapChain::createImageViews(Image& imageClass) {
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		swapChainImageViews[i] = imageClass.createImageView(swapChainImages[i],
			swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

void SwapChain::cleanupSwapChain(AttachmentManager& attachmentManager) {
	vkDestroyImage(m_Devices.device, attachmentManager.m_GPUColor.image(), nullptr);
	vkDestroyImageView(m_Devices.device, attachmentManager.m_GPUColor.view(), nullptr);
	vkFreeMemory(m_Devices.device, attachmentManager.m_GPUColor.memory(), nullptr);
	vkDestroyImageView(m_Devices.device, attachmentManager.m_GPUDepth.view(), nullptr);
	vkDestroyImage(m_Devices.device, attachmentManager.m_GPUDepth.image(), nullptr);
	vkFreeMemory(m_Devices.device, attachmentManager.m_GPUDepth.memory(), nullptr);

	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(m_Devices.device, framebuffer, nullptr);
	}

	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(m_Devices.device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_Devices.device, swapChain, nullptr);
}

void SwapChain::createSyncObjects() {

	imageAvailableSemaphores.resize(Constants::MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(swapChainImages.size());
	imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);
	inFlightFences.resize(Constants::MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (size_t i = 0; i < Constants::MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(m_Devices.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(m_Devices.device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create per-frame sync objects!");
		}
	}
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		if (vkCreateSemaphore(m_Devices.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create per-image sync objects!");
		}
	}
}

void SwapChain::recreateSwapChain(GLFWwindow* window, Image& imageClass, AttachmentManager& attachmentManager, RenderPass& renderPass) {
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_Devices.device);

	cleanupSwapChain(attachmentManager);

	createSwapChain();
	createImageViews(imageClass);
	attachmentManager.createColorResources();
	attachmentManager.createDepthResources();
	renderPass.createFramebuffers(attachmentManager.m_GPUColor.view(), attachmentManager.m_GPUDepth.view());
	createSyncObjects();
}