#pragma once
#include "Shaders.h"
#include "SwapChain.h"
#include "DescriptorSetLayout.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
class GraphicsPipeline
{
public:
	GraphicsPipeline(Shaders& shaders, VkDevice& device, SwapChain& swapChain, VkSampleCountFlagBits& msaaSamples,
		DescriptorSetLayout& descriptorSetLayout, VkRenderPass& renderPass);
	void createGraphicsPipeline();
	void createMandelbulbComputePipeline();
	void createMandelbulbGraphicsPipeline();

	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	VkPipelineLayout mandelbulbPipelineComputeLayout;
	VkPipeline mandelbulbComputePipeline;

	VkPipelineLayout mandelbulbPipelineGraphicsLayout;
	VkPipeline mandelbulbGraphicsPipeline;

private:
	Shaders& m_Shaders;
	VkDevice& m_device;
	SwapChain& m_SwapChain;
	VkSampleCountFlagBits& m_msaaSamples;
	DescriptorSetLayout& m_DescriptorSetLayout;
	VkRenderPass& m_renderPass;

};

