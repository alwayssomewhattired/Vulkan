#pragma once

#include <array>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <stdexcept>

#include "Devices.h"
#include "SwapChain.h"
#include "AttachmentManager.h"

class RenderPass
{
public:
	RenderPass(Devices& devices, SwapChain& swapChain);

	void createFramebuffers(VkImageView& colorImageView, VkImageView& depthImageView);

	void createRenderPass(AttachmentManager& attachmentManager);

	VkRenderPass renderPass;

private:
	Devices& m_Devices;
	SwapChain& m_SwapChain;
};

