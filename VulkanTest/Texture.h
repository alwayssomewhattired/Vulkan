#pragma once

#include <tiny_gltf.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <utility>

#include "glm_config.h"
#include "GPUTexture.h"

#include <stdexcept>

#include <stb_image.h>


class Image;
class Buffer;
class Devices;
class CommandBuffer;
class ItemInterface;

class Texture
{
public:
	Texture(Buffer& buffer, Image& image, Devices& devices, CommandBuffer& commandBuffer);
	
	// | central texture storage
	// | (use indices from item class to access elements)
	std::vector<GPUTexture> m_gpuTextures;

	// | organised, unique gltf-index to gpu-texture-index cache 
	std::unordered_map<int, int> m_gltfToGpuTexture;

	// - i think this should be non-copyable as well
	struct GLTFMaterial {
		int baseColorTex;
		int normalTex;
		glm::vec4 baseColorFactor;
	};

	// | updates outTexture struct. does more than just create images...
	// | must be called if supplying an independent texture
	// * isDefault : if true, creates default 1x1 texture
	// * texturePath(optional) : path to seperate texture
	// * pixelData(optional) : for pixel data of texture
	// * outTexture(optional) : for struct of attached texture
	void createTextureImage(const bool isDefault, const std::string& texturePath, const uint8_t* pixelData, 
		GPUTexture& outTexture, const VkFormat& format, int width, int height);

	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	uint32_t mipLevels;

	void buildGPUMaterial(tinygltf::Model& model, int& materialIndex, ItemInterface& classReference, const uint32_t primitiveIdx);

private:
	Buffer& m_Buffer;
	Image& m_Image;
	Devices& m_Devices;
	CommandBuffer& m_CommandBuffer;
	void createTextureSampler(const uint32_t& mipLevels, GPUTexture& outTex);
	int uploadGltfTextureToVulkan(tinygltf::Model& model, int& textureIndex, ItemInterface& classReference,
		const VkFormat& format);

	void createDefaultTextures();

	int getOrCreateGpuTexture(
		tinygltf::Model& model,
		int gltfTexIndex,
		ItemInterface& classReference,
		VkFormat format);
};

