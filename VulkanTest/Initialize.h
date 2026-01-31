#pragma once
#include <memory>

#include "ValidationLayers.h"
#include "Devices.h"
#include "SwapChain.h"
#include "DescriptorSetLayout.h"
#include "GraphicsPipeline.h"
#include "Shaders.h"
#include "HostToDevice.h"
#include "items/Home.h"
#include "items/SilentHill3Game.h"

// | Initializes unique ptrs
class Initialize
{
	std::unique_ptr<ValidationLayers> m_ValidationLayers = nullptr;

	std::unique_ptr<Devices> m_devices = nullptr;
	std::unique_ptr<VkDevice> m_device = nullptr;
	std::unique_ptr<VkPhysicalDevice> m_physicalDevice = nullptr;
	std::unique_ptr<VkSampleCountFlagBits> m_msaaSamples = nullptr;
	std::unique_ptr<Devices::QueueFamilyIndices> m_QueueFamilyIndices = nullptr;
	std::unique_ptr<Devices::SwapChainSupportDetails> m_SwapChainSupportDetails = nullptr;
	std::unique_ptr<VkQueue> m_graphicsQueue = nullptr;

	std::unique_ptr<HostToDevice> m_HostToDevice = nullptr;

	std::unique_ptr<SwapChain> m_SwapChain = nullptr;

	std::unique_ptr<DescriptorSetLayout> m_DescriptorSetLayout = nullptr;

	std::unique_ptr<Shaders> m_Shaders = nullptr;

	std::unique_ptr<GraphicsPipeline> m_GraphicsPipeline = nullptr;

	std::unique_ptr<Home> m_home = nullptr;
	std::unique_ptr<SilentHill3Game> m_SilentHill3Game = nullptr;
};

