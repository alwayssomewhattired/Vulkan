#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Image.h"
#include "SwapChain.h"
#include "Interface.h"
#include "Devices.h"

class StorageImageManager
{
public:

	StorageImageManager(Image& image, SwapChain& swapChain, Devices& devices);

	void createStorageImageResources();

	void createMandelbulbSampler();

	struct GPUStorageImage : ImageBundle {
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

	GPUStorageImage m_GPUStorageImage{};

	VkSampler mandelbulbSampler;

private:
	
	Image& m_Image;
	SwapChain& m_SwapChain;
	Devices& m_Devices;
};

