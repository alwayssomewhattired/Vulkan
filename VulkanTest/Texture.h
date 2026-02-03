#pragma once

#include <tiny_gltf.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <stdexcept>

#include "Image.h"
#include "Buffer.h"
#include "Devices.h"
#include "CommandBuffer.h"
#include "Constants.h"

class Texture
{
public:
	Texture(Buffer& buffer, Image& image, Devices& devices, CommandBuffer& commandBuffer);
	// * isDefault : if true, creates default 1x1 texture
	struct GPUTexture {
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
		VkSampler sampler;
	};

	void createTextureImage(const bool isDefault, const std::string& texturePath, const uint8_t* pixelData, 
		GPUTexture& outTexture);
	void createTextureImageView();
	void createTextureSampler();
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	uint32_t mipLevels;
	VkImage textureImage;
	VkImageView textureImageView;
	VkDeviceMemory textureImageMemory;
	VkSampler textureSampler;


	struct GPUMaterial {
		int baseColorTex;
		int normalTex;
		glm::vec4 baseColorFactor;
	};

	std::vector<GPUTexture> gpuTextures;
	std::vector<GPUMaterial> gpuMaterials;

	void uploadGltfTextureToVulkan(tinygltf::Model& model, int& textureIndex);

	void buildGPUMaterial(tinygltf::Model& model, int& materialIndex);

private:
	Buffer& m_Buffer;
	Image& m_Image;
	Devices& m_Devices;
	CommandBuffer& m_CommandBuffer;
};

