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

// | model loader AND safe-room model class
// - yeah, not good. decouple safe-room model from model loading 
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

	//void uploadGltfTextureToVulkan(tinygltf::Model& model, int& textureIndex, ItemInterface& classReference);
	//void buildGPUMaterial(tinygltf::Model& model, int& materialIndex, ItemInterface& classReference);
	//GPUTexture create1x1Texture(const uint8_t* rgbaPixel, VkFormat format);
	void fileDebug(const tinygltf::Model& model);

	// | staging mesh vertices
	//std::vector<Vertex> vertices;
	//std::vector<uint32_t> indices;
};