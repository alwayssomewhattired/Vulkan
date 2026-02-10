#pragma once

#include <tiny_gltf.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "glm_config.h"

#include <stdexcept>
//#include <glm/fwd.hpp>
//#include <glm/glm.hpp>
#include <stb_image.h>

//#include "Image.h"
//#include "Buffer.h"
//#include "Devices.h"
//#include "CommandBuffer.h"
//#include "Constants.h"

//#include "Items/ItemInterface.h"

class Image;
class Buffer;
class Devices;
class CommandBuffer;
class ItemInterface;

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
		GPUTexture& outTexture, const VkFormat& format);
	//void createTextureImageView();
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	uint32_t mipLevels;
	// - get rid of these texture variables. we use our texture struct from now on
	VkImage textureImage;
	VkImageView textureImageView;
	VkDeviceMemory textureImageMemory;
	VkSampler textureSampler;


	struct GPUMaterial {
		int baseColorTex;
		int normalTex;
		glm::vec4 baseColorFactor;
	};

	// don't store textures/materials in this class. store it in the item's class
	//std::vector<GPUTexture> gpuTextures;
	//std::vector<GPUMaterial> gpuMaterials;


	void buildGPUMaterial(tinygltf::Model& model, int& materialIndex, ItemInterface& classReference);

private:
	Buffer& m_Buffer;
	Image& m_Image;
	Devices& m_Devices;
	CommandBuffer& m_CommandBuffer;
	void createTextureSampler(const uint32_t& mipLevels, Texture::GPUTexture& outTex);
	int uploadGltfTextureToVulkan(tinygltf::Model& model, int& textureIndex, ItemInterface& classReference,
		const VkFormat& format);
};

