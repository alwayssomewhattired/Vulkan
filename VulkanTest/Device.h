#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

class Device {
public:

	Device(VkSurfaceKHR& surface);
	void createLogicalDevice();
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	struct QueueFamilyIndices;
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	struct SwapChainSupportDetails;

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	VkDevice device;
	VkQueue presentQueue;
	VkQueue graphicsQueue;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

private:
	VkSurfaceKHR& m_surface;
};