#pragma once
#include "Shaders.h"
#include "SwapChain.h"
#include "descriptor_sets/DescriptorSetLayout.h"
#include "UniformBuffer.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <iostream>
class GraphicsPipeline
{
public:
	GraphicsPipeline(Shaders& shaders, VkDevice device, SwapChain& swapChain, VkSampleCountFlagBits msaaSamples,
		descriptorSetLayout& descriptorSetLayout, VkRenderPass& renderPass);
	void createGraphicsPipeline(UniformBuffer& uniformBuffer);
	void createMandelbulbComputePipeline();
	void createMandelbulbGraphicsPipeline();

	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	VkPipelineLayout mandelbulbPipelineComputeLayout;
	VkPipeline mandelbulbComputePipeline;

	VkPipelineLayout mandelbulbPipelineGraphicsLayout;
	VkPipeline mandelbulbGraphicsPipeline;

private:
	VkSampleCountFlagBits m_msaaSamples;
	VkDevice m_device;
	Shaders& m_Shaders;
	SwapChain& m_SwapChain;
	descriptorSetLayout& m_descriptorSetLayout;
	VkRenderPass& m_renderPass;

};

