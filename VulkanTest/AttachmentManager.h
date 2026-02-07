#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "SwapChain.h"
#include "Image.h"
#include "Interface.h"
#include "Devices.h"

class AttachmentManager
{
public:
	AttachmentManager(SwapChain& swapChain, VkSampleCountFlagBits& msaaSamples, Image& image, Devices& devices);

	void createColorResources();

	void createDepthResources();

	VkFormat findDepthFormat();

	bool hasStencilComponent(VkFormat format);

	struct GPUDepth : ImageBundle {
		VkImage m_image;
		VkDeviceMemory m_memory;
		VkImageView m_view;

		VkImage& image() override {
			return m_image;
		}

		VkDeviceMemory& memory() override {
			return m_memory;
		}

		VkImageView& view() override {
			return m_view;
		}
	};

	GPUDepth m_GPUDepth{};

	struct GPUColor : ImageBundle {
		VkImage m_image;
		VkDeviceMemory m_memory;
		VkImageView m_view;

		VkImage& image() override {
			return m_image;
		}

		VkDeviceMemory& memory() override {
			return m_memory;
		}

		VkImageView& view() override {
			return m_view;
		}
	};

	GPUColor m_GPUColor{};

private:
	SwapChain& m_SwapChain;
	VkSampleCountFlagBits& m_msaaSamples;
	Image& m_Image;
	Devices& m_Devices;
};

