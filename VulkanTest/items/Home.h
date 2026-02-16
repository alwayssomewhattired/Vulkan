#pragma once


#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>

#include "ItemInterface.h"
#include "../Vertex.h"
#include "../glm_config.h"
#include "../Texture.h"
#include "../GPUTexture.h"

class Home : public ItemInterface
{

public:
	glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -3));
	
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexMemory;
	
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexMemory;
	
	size_t m_vertexCount;
	uint32_t m_indexCount;

	VkIndexType m_indexType;

	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;

	std::vector<Texture::GLTFMaterial> m_gltfMaterials;

	std::string m_optionalTexturePath = "textures/Metal055C_8K-PNG_Color.png";

	std::vector<VkDescriptorSet> m_descriptorSets;

	// | begin item interface block
	VkBuffer& vertexBuffer() override {
		return m_vertexBuffer;
	};

	VkDeviceMemory& vertexMemory() override {
		return m_vertexMemory;
	}

	VkBuffer& indexBuffer() override {
		return m_indexBuffer;
	};

	VkDeviceMemory& indexMemory() override {
		return m_indexMemory;
	};
	
	size_t& vertexCount() override {
		return m_vertexCount;
	}

	uint32_t& indexCount() override {
		return m_indexCount;
	}
		
	VkIndexType& indexType() override {
		return m_indexType;
	}

	std::vector<Vertex>& vertices() override {
		return m_vertices;
	}

	std::vector<uint32_t>& indices() override {
		return m_indices;
	}

	std::vector<Texture::GLTFMaterial>& gltfMaterials() override {
		return m_gltfMaterials;
	}

	std::string& optionalTexturePath() override {
		return m_optionalTexturePath;
	}

	std::vector<VkDescriptorSet>& descriptorSets() override {
		return m_descriptorSets;
	}
};

