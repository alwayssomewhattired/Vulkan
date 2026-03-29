#include "CommandBuffer.h"

#include "SwapChain.h"
#include "Devices.h"
#include "CommandPool.h"
#include "GraphicsPipeline.h"
#include "RenderTarget.h"
#include "items/ItemInterface.h"
#include "items/Home.h"
#include "items/SilentHill3Game.h"
#include "descriptor_sets/DescriptorSet.h"
#include "Constants.h"
#include "UniformBuffer.h"
#include "Buffer.h"

CommandBuffer::CommandBuffer(VkCommandPool& commandPool, Devices& devices, SwapChain& swapChain) : 
	m_commandPool(commandPool), m_devices(devices), m_SwapChain(swapChain) {};

VkCommandBuffer CommandBuffer::beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_devices.device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void CommandBuffer::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_devices.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_devices.graphicsQueue);

	vkFreeCommandBuffers(m_devices.device, m_commandPool, 1, &commandBuffer);
}

void CommandBuffer::createCommandBuffers(CommandPool& commandPool) {

	commandBuffers.resize(m_SwapChain.swapChainImages.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(m_devices.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate command buffers");

}

void CommandBuffer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, VkRenderPass& renderPass,
	GraphicsPipeline& graphicsPipeline, std::vector<ItemInterface*>& items, DescriptorSet& descriptorSet, 
	const uint32_t currentFrame, VkImage& storageImage, ItemInterface& triangleClass, UniformBuffer& uniformBuffer,
	MaterialDescriptorSet& _materialDescriptorSet, Buffer& buffer) {

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		throw std::runtime_error("failed to begin recording command buffer!");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = m_SwapChain.swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_SwapChain.swapChainExtent;

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(m_SwapChain.swapChainExtent.width);
	viewport.height = static_cast<float>(m_SwapChain.swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapChain.swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	auto& renderTriangle = g_renderTarget.renderTriangle;
	auto& renderMandelbulb = g_renderTarget.renderMandelbulb;
	auto& mandelbulbComputeDescriptorSets = descriptorSet.mandelbulbComputeDescriptorSets;
	auto& mandelbulbGraphicsDescriptorSets = descriptorSet.mandelbulbGraphicsDescriptorSets;
	

	// model
	if (!renderTriangle && !renderMandelbulb) {

		if (items.empty()) {
			std::cout << "no items... this is not good \n";
			return;
		}

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.graphicsPipeline);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipelineLayout, 0, 1,
			&descriptorSet.globalDescriptorSets[currentFrame], 0, nullptr);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipelineLayout, 3, 1,
			&_materialDescriptorSet.materialDescriptorSet, 0, nullptr);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipelineLayout, 1, 1,
			&descriptorSet.meshMaterialDescriptorSet, 0, nullptr);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipelineLayout, 2, 1,
			&descriptorSet.animationDescriptorSet, 0, nullptr);

		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer.globalVertexBuffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, buffer.globalIndexBuffer, 0, Constants::INDEX_TYPE);
		
		for (size_t itemIndex = 0; itemIndex < items.size(); itemIndex++) {
			auto& item = items[itemIndex];
			auto& mesh = item->meshData;

			struct PushConstants {
				glm::mat4 model;
				glm::vec4 lightPos;
				glm::uint materialIndex;
				glm::uint boneOffset;
				glm::uint boneCount;
			};

			PushConstants pc{};

			pc.boneOffset = item->skeleton.offset;
			pc.boneCount = item->skeleton.count;

			for (int i = 0; i < mesh.indexCount.size(); i++) { // mesh iteration (per primitive)

				pc.model = item->modelMatrix.model;
				pc.lightPos = { 0.0f, 2.0f, 3.5f, 1.0f };
				pc.materialIndex = item->materialData.materialIndices[i];
				glm::vec4 lightPos = { 0.0f, 2.0f, 3.5f, 1.0f };

				vkCmdPushConstants(
					commandBuffer,
					graphicsPipeline.pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(PushConstants),
					&pc
				);

				vkCmdDrawIndexed(commandBuffer, mesh.indexCount[i], 1, mesh.firstIndex[i], mesh.vertexOffset[i], 0);
			}
		}
	}
	// mandelbulb
	else if (!renderTriangle && renderMandelbulb) {
		//
		// COMPUTE PHASE
		//
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, graphicsPipeline.mandelbulbComputePipeline);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_COMPUTE,
			graphicsPipeline.mandelbulbPipelineComputeLayout,
			0,
			1,
			&mandelbulbComputeDescriptorSets[currentFrame],
			0,
			nullptr
		);

		// Ensure image in GENERAL layout
		VkImageMemoryBarrier barrierToGeneral = {};
		barrierToGeneral.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrierToGeneral.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrierToGeneral.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		barrierToGeneral.srcAccessMask = 0;
		barrierToGeneral.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrierToGeneral.image = storageImage;
		barrierToGeneral.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrierToGeneral.subresourceRange.levelCount = 1;
		barrierToGeneral.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0, 0, nullptr, 0, nullptr,
			1, &barrierToGeneral
		);

		// Dispatch
		uint32_t groupX = (m_SwapChain.swapChainExtent.width + 15) / 16;
		uint32_t groupY = (m_SwapChain.swapChainExtent.height + 15) / 16;
		vkCmdDispatch(commandBuffer, groupX, groupY, 1);

		// Make writes visible to fragment shader
		VkImageMemoryBarrier barrierToRead = {};
		barrierToRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrierToRead.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		barrierToRead.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		//barrierToRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrierToRead.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrierToRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrierToRead.image = storageImage;
		barrierToRead.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrierToRead.subresourceRange.levelCount = 1;
		barrierToRead.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr,
			1, &barrierToRead
		);

		//
		// GRAPHICS PHASE
		//
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.mandelbulbGraphicsPipeline);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			graphicsPipeline.mandelbulbPipelineGraphicsLayout,
			0, 1,
			&mandelbulbGraphicsDescriptorSets[currentFrame],
			0, nullptr
		);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}
	// triangle
	else if (renderTriangle && !renderMandelbulb) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.graphicsPipeline);

		//VkBuffer triangleVertexBuffers[] = { triangleClass.meshData.vertexBuffer};
		VkDeviceSize triangleOffsets[] = { 0 };
		//vkCmdBindVertexBuffers(commandBuffer, 0, 1, triangleVertexBuffers, triangleOffsets);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.pipelineLayout, 0, 1,
			&triangleClass.materialData.descriptorSets[currentFrame], 0, nullptr);

		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		throw std::runtime_error("failed to record command buffer");
}

void CommandBuffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);
}

