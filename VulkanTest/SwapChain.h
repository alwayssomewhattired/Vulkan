#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>

#include "Devices.h"
//#include "AttachmentManager.h"
//#include "Image.h"
//#include "RenderPass.h"

struct GLFWwindow;

class Devices;
class AttachmentManager;
class Image;
class RenderPass;

class SwapChain
{
public:
	SwapChain(Devices& devices, VkSurfaceKHR surface);

	void createSwapChain();

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	//size_t swapChainImageCount = swapChainImages.size();
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkImageView> swapChainImageViews;

	void cleanupSwapChain(AttachmentManager& attachmentManager);

	void recreateSwapChain(GLFWwindow* window, Image& imageClass, AttachmentManager& attachmentManager, RenderPass& renderPass);
	void createImageViews(Image& imageClass);
	void createSyncObjects();

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> imagesInFlight;
	std::vector<VkFence> inFlightFences;

private:
	Devices& m_Devices;
	Devices::SwapChainSupportDetails m_SwapChainSupportDetails;
	VkSurfaceKHR m_surface;

};

