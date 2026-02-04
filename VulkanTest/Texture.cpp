#include "Texture.h"


Texture::Texture(Buffer& buffer, Image& image, Devices& devices, CommandBuffer& commandBuffer) : 
	m_Buffer(buffer), m_Image(image), m_Devices(devices), m_CommandBuffer(commandBuffer) {}

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

// | updates outTexture struct. does more than just create images...
// * isDefault : if true, creates default 1x1 texture
// * texturePath(optional) : path to seperate texture
// * pixelData(optional) : for pixel data of seperate texture
// * outTexture(optional) : for struct of attached texture
// - note: this now also creates views and sampler. make the other calls to them two functinos  after this function go away. 
// possibly make these two 
// member functions private.
void Texture::createTextureImage(const bool isDefault, const std::string& texturePath, const uint8_t* pixelData, 
	GPUTexture& outTexture, const VkFormat& format) {
	int texWidth, texHeight, texChannels;

	// - double check if this is correct
	if (isDefault) {
		texWidth = 1;
		texHeight = 1;
		texChannels = 4;
	}
	else if (!texturePath.empty())
		pixelData = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!isDefault)
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

	if (!isDefault)
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

	m_Image.createImageView(outTexture.image, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, outTexture);
	createTextureSampler(mipLevels, outTexture);

}

// | not needed anymore. createImageView does everything for us
//void Texture::createTextureImageView() {
//	textureImageView = m_Image.createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
//}

void Texture::createTextureSampler(const uint32_t& mipLevels, Texture::GPUTexture& outTex) {
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

void Texture::uploadGltfTextureToVulkan(tinygltf::Model& model, int& textureIndex, ItemInterface& classReference) {
	const tinygltf::Texture& texture = model.textures[textureIndex];
	const tinygltf::Image& image = model.images[texture.source];
	const tinygltf::Sampler& sampler = model.samplers[texture.sampler];

	// do all the stuff here 

	classReference.gpuTextures()[textureIndex];
}

void Texture::buildGPUMaterial(tinygltf::Model& model, int& materialIndex, ItemInterface& classReference) {
	const tinygltf::Material& material = model.materials[materialIndex];

	GPUMaterial mat{};

	mat.baseColorTex = material.pbrMetallicRoughness.baseColorTexture.index;

	mat.normalTex = material.normalTexture.index;

	mat.baseColorFactor = glm::make_vec4(material.pbrMetallicRoughness.baseColorFactor.data());

	// handle missing textures
	if (mat.baseColorTex < 0)
		mat.baseColorTex = Constants::DEFAULT_WHITE_TEXTURE_INDEX;

	if (mat.normalTex < 0)
		mat.normalTex = Constants::DEFAULT_NORMAL_TEXTURE_INDEX;

	classReference.gpuMaterials()[materialIndex] = mat;
}