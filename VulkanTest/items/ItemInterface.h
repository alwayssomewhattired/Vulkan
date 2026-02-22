#pragma once
#include <glm/fwd.hpp>

#include "../glm_config.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <vector>

#include "../Vertex.h"
#include "../Texture.h"
#include "../GPUTexture.h"

// | blueprint for all items
struct ItemInterface {

	virtual std::vector<VkBuffer>& vertexBuffer() = 0;
	virtual std::vector<VkDeviceMemory>& vertexMemory() = 0;

	virtual std::vector<VkBuffer>& indexBuffer() = 0;
	virtual std::vector<VkDeviceMemory>& indexMemory() = 0;

	virtual std::vector<size_t>& vertexCount() = 0;
	virtual std::vector<uint32_t>& indexCount() = 0;

	virtual VkIndexType& indexType() = 0;

	virtual std::vector<std::vector<Vertex>>& vertices() = 0;
	virtual std::vector<std::vector<uint32_t>>& indices() = 0;

	// | gltf material indices
	virtual std::vector<Texture::GLTFMaterial>& gltfMaterials() = 0;

	// | primitive-material indices
	virtual std::vector<int>& gltfPrimitiveMaterialIndices() = 0;

	virtual std::string& optionalTexturePath() = 0;

	virtual std::vector<VkDescriptorSet>& descriptorSets() = 0;

	virtual ~ItemInterface() = default;
};