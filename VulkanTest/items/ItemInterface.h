#pragma once
#include <glm/fwd.hpp>
//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>

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

	virtual VkBuffer& vertexBuffer() = 0;
	virtual VkDeviceMemory& vertexMemory() = 0;

	virtual VkBuffer& indexBuffer() = 0;
	virtual VkDeviceMemory& indexMemory() = 0;

	virtual size_t& vertexCount() = 0;
	virtual uint32_t& indexCount() = 0;

	virtual VkIndexType& indexType() = 0;

	virtual std::vector<Vertex>& vertices() = 0;
	virtual std::vector<uint32_t>& indices() = 0;

	// | gltf material indices
	virtual std::vector<Texture::GLTFMaterial>& gltfMaterials() = 0;

	virtual std::string& optionalTexturePath() = 0;

	virtual std::vector<VkDescriptorSet>& descriptorSets() = 0;

	virtual ~ItemInterface() = default;
};