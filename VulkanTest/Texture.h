#pragma once

#include <tiny_gltf.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <utility>

#include "glm_config.h"
#include "GPUTexture.h"

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

	//// | non-copyabe
	//struct GPUTexture {
	//	VkImage image = VK_NULL_HANDLE;
	//	VkDeviceMemory memory = VK_NULL_HANDLE;
	//	VkImageView view = VK_NULL_HANDLE;
	//	VkSampler sampler = VK_NULL_HANDLE;

	//	// | This code block is for making our struct non-copyable

	//	GPUTexture() = default;

	//	// | deletes copy constructor
	//	GPUTexture(const GPUTexture&) = delete;

	//	// | deletes copy assignment operator
	//	GPUTexture& operator=(const GPUTexture&) = delete;

	//	// | move constructor
	//	GPUTexture(GPUTexture&& other) noexcept {
	//		*this = std::move(other);
	//	}

	//	// | move assignment operator
	//	GPUTexture& operator=(GPUTexture&& other) noexcept {

	//		// | transfers handles
	//		image = other.image;
	//		memory = other.memory;
	//		view = other.view;
	//		sampler = other.sampler;

	//		// | nulls out the source
	//		other.image = VK_NULL_HANDLE;
	//		other.memory = VK_NULL_HANDLE;
	//		other.view = VK_NULL_HANDLE;
	//		other.sampler = VK_NULL_HANDLE;

	//	}
	//	
	//};


	// - i think this should be non-copyable as well
	struct GPUMaterial {
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
		GPUTexture& outTexture, const VkFormat& format);

	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	uint32_t mipLevels;

	void buildGPUMaterial(tinygltf::Model& model, int& materialIndex, ItemInterface& classReference);

private:
	Buffer& m_Buffer;
	Image& m_Image;
	Devices& m_Devices;
	CommandBuffer& m_CommandBuffer;
	void createTextureSampler(const uint32_t& mipLevels, GPUTexture& outTex);
	int uploadGltfTextureToVulkan(tinygltf::Model& model, int& textureIndex, ItemInterface& classReference,
		const VkFormat& format);
};

