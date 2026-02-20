#pragma once

#include <tiny_gltf.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <vector>

#include "items/ItemInterface.h"
#include "Vertex.h"
#include "Buffer.h"
#include "CommandBuffer.h"
#include "Texture.h"
#include "GPUTexture.h"

// | model loader
class ModelLoad
{
public:

	ModelLoad(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue,
		Buffer& buffer, CommandBuffer& commandBuffer, Texture& texture
		);


	// | loads model and fills in class
	void loadModel(const std::string& path, ItemInterface& classReference);
	

private:
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;


	Buffer& m_Buffer;
	CommandBuffer& m_CommandBuffer;
	Texture& m_Texture; 

	void modelFileParse(tinygltf::Model& model, const tinygltf::Primitive& primitive, size_t& vertexCount, 
		std::vector<Vertex>& vertices,
		VkIndexType& indexType, std::vector<uint32_t>& indices, ItemInterface& classReference, const uint32_t primitiveIdx,
		const uint32_t primitivesSize);
	void fileDebug(const tinygltf::Model& model);
};