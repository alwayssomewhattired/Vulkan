#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>

#include "Devices.h"
class SwapChain
{
public:
	SwapChain(Devices& devices, VkSurfaceKHR& surface);

	void createSwapChain();

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	size_t swapChainImageCount = swapChainImages.size();
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkImageView> swapChainImageViews;

private:
	Devices& m_Devices;
	Devices::SwapChainSupportDetails& m_SwapChainSupportDetails;
	VkSurfaceKHR& m_surface;
};

