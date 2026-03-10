#include "Texture.h"

#include "Image.h"
#include "Buffer.h"
#include "Devices.h"
#include "CommandBuffer.h"
#include "Constants.h"

#include "Items/ItemInterface.h"

Texture::Texture(Buffer& buffer, Image& image, Devices& devices, CommandBuffer& commandBuffer) : 
	m_Buffer(buffer), m_Image(image), m_Devices(devices), m_CommandBuffer(commandBuffer) 
{
	createDefaultTextures();
}


void Texture::createTextureImage(const bool isDefault, const std::string& texturePath, const uint8_t* pixelData, 
	GPUTexture& outTexture, const VkFormat& format, int texWidth, int texHeight) {

	int texChannels = 4;

	if (!texturePath.empty())
		pixelData = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (pixelData)
		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
	else
		mipLevels = 1;

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixelData)
		throw std::runtime_error("failed to load texture image!");

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	m_Buffer.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);
	
	void* data;
	vkMapMemory(m_Devices.device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixelData, static_cast<size_t>(imageSize));
	vkUnmapMemory(m_Devices.device, stagingBufferMemory);

	if (!texturePath.empty())
		stbi_image_free((void*)pixelData);

	m_Image.createImage(
		texWidth, 
		texHeight, 
		mipLevels, 
		VK_SAMPLE_COUNT_1_BIT, 
		format, 
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | (isDefault ? 0 : VK_IMAGE_USAGE_TRANSFER_SRC_BIT),
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
		outTexture.image, 
		outTexture.memory
	);

	m_Image.transitionImageLayout(outTexture.image, format, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		mipLevels);

	m_Image.copyBufferToImage(stagingBuffer, outTexture.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	if (!isDefault) {
		generateMipmaps(outTexture.image, format, texWidth, texHeight, mipLevels);
	} else {
		m_Image.transitionImageLayout(
			outTexture.image,
			format,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			1
		);
	}

	vkDestroyBuffer(m_Devices.device, stagingBuffer, nullptr);
	vkFreeMemory(m_Devices.device, stagingBufferMemory, nullptr);

	outTexture.view = m_Image.createImageView(outTexture.image, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
	createTextureSampler(mipLevels, outTexture);

}

void Texture::createTextureSampler(const uint32_t& mipLevels, GPUTexture& outTex) {
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
	samplerInfo.mipLodBias = 0.0f;

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(m_Devices.physicalDevice, &properties);
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	if (vkCreateSampler(m_Devices.device, &samplerInfo, nullptr, &outTex.sampler) != VK_SUCCESS)
		throw std::runtime_error("failed to create texture sampler!");
}

int Texture::uploadAssimpTextureToVulkan(const aiScene* scene, const aiString& path, ItemInterface& classReference, 
	const VkFormat& format) {

	GPUTexture gpuTex(m_Devices.device);

	const aiTexture* embedded = scene->GetEmbeddedTexture(path.C_Str());

	if (embedded)
	{
		// | EMBEDDED TEXTURE

		if (embedded->mHeight == 0)
		{
			// | COMPRESSED (PNG/JPG in memory)

			int texWidth, texHeight, texChannels;

			stbi_uc* pixels = stbi_load_from_memory(
				reinterpret_cast<const stbi_uc*>(embedded->pcData),
				embedded->mWidth,
				&texWidth,
				&texHeight,
				&texChannels,
				STBI_rgb_alpha
			);

			if (!pixels)
				throw std::runtime_error("failed to decode embedded texture");

			createTextureImage(false, classReference.optionalTexturePath(), pixels,
				gpuTex, format, texWidth, texHeight);

			stbi_image_free(pixels);
		}
		else
		{
			// | RAW RGBA pixels

			createTextureImage(false, classReference.optionalTexturePath(), reinterpret_cast<const uint8_t*>(embedded->pcData),
				gpuTex, format, embedded->mWidth, embedded->mHeight);
		}
	}
	else
	{
		// | EXTERNAL FILE PATH

		createTextureImage(false, path.C_Str(), nullptr, gpuTex, format, 0, 0);
	}

	m_gpuTextures.push_back(std::move(gpuTex));

	return static_cast<int>(m_gpuTextures.size() - 1);

}


int Texture::getOrCreateGpuTexture(
	const aiScene* scene,
	aiMaterial* material,
	aiTextureType type,
	ItemInterface& classReference,
	VkFormat format)
{
	aiString path;

	if (material->GetTexture(type, 0, &path) != AI_SUCCESS) {
		return Constants::DEFAULT_WHITE_TEXTURE_INDEX;
	}

	return uploadAssimpTextureToVulkan(scene, path, classReference, format);
}

void Texture::buildGPUMaterial(
	const aiScene* scene,
	aiMaterial* material,
	unsigned int materialIndex,
	ItemInterface& classReference, const uint32_t primitiveIdx)
{
	GLTFMaterial mat{};

	// | BASE COLOR FACTOR

	aiColor4D baseColor(1, 1, 1, 1);

	if (material->Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS ||
		material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS)
	{
		glm::vec4 baseColorFactor(
			baseColor.r,
			baseColor.g,
			baseColor.b,
			baseColor.a
		);

		mat.baseColorFactor = baseColorFactor;
	}
	else
	{
		mat.baseColorFactor = glm::vec4(1.0f);
	}

	aiString path;

	// | BASE COLOR TEXTURE (srgb)

	if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &path) == AI_SUCCESS ||
		material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
	{
		mat.baseColorTex = getOrCreateGpuTexture(scene, material, aiTextureType_BASE_COLOR, classReference, 
			VK_FORMAT_R8G8B8A8_SRGB);
	}

	// | NORMAL MAP (unorm)

	if (material->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
	{

		mat.normalTex = getOrCreateGpuTexture(scene, material, aiTextureType_NORMALS, classReference, VK_FORMAT_R8G8B8A8_UNORM);
	}

	classReference.gltfMaterials()[materialIndex] = mat;

}


void Texture::createDefaultTextures() {


	// default base-color (default white)
	GPUTexture whiteTex(m_Devices.device);
	createTextureImage(true, "", Constants::WHITE_PIXEL, whiteTex, VK_FORMAT_R8G8B8A8_SRGB, 1, 1);
	Constants::DEFAULT_WHITE_TEXTURE_INDEX = m_gpuTextures.size();
	m_gpuTextures.push_back(std::move(whiteTex));

	// default (black)
	GPUTexture blackTex(m_Devices.device);
	createTextureImage(true, "", Constants::BLACK_PIXEL, blackTex, VK_FORMAT_R8G8B8A8_SRGB, 1, 1);
	Constants::DEFAULT_BLACK_TEXTURE_INDEX = m_gpuTextures.size();
	m_gpuTextures.push_back(std::move(blackTex));

	// | default normals (default flat)
	GPUTexture normalTex(m_Devices.device);
	createTextureImage(true, "", Constants::NORMAL_PIXEL, normalTex, VK_FORMAT_R8G8B8A8_UNORM, 1, 1);
	Constants::DEFAULT_NORMAL_TEXTURE_INDEX = m_gpuTextures.size();
	m_gpuTextures.push_back(std::move(normalTex));

}



void Texture::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(m_Devices.physicalDevice, imageFormat, &formatProperties);
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
		throw std::runtime_error("texture image format does not support linear blitting!");

	VkCommandBuffer commandBuffer = m_CommandBuffer.beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR
		);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	m_CommandBuffer.endSingleTimeCommands(commandBuffer);
}