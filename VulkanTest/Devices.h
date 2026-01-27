#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#include <cstdint>

class Devices {
public:

	Devices(VkSurfaceKHR& surface, VkInstance& instance, GLFWwindow* window);
	void createLogicalDevice();
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	VkSampleCountFlagBits getMaxUsableSampleCount();

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	QueueFamilyIndices m_QueueFamilyIndices;
	void findQueueFamilies(VkPhysicalDevice device);

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	SwapChainSupportDetails m_SwapChainSupportDetails;
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	VkDevice device;
	VkQueue presentQueue;
	VkQueue graphicsQueue;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

private:
	VkSurfaceKHR& m_surface;
	VkInstance& m_instance;
	GLFWwindow* m_window;
};