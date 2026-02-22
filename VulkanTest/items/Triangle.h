
#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <vector>
#include <string>

#include "ItemInterface.h"
#include "../Vertex.h"
#include "../glm_config.h"
#include "../Texture.h"
#include "../GPUTexture.h"

class Triangle : public ItemInterface
{

public:
	glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -3));

	std::vector<VkBuffer> m_vertexBuffer;
	std::vector<VkDeviceMemory> m_vertexMemory;

	std::vector<VkBuffer> m_indexBuffer;
	std::vector<VkDeviceMemory> m_indexMemory;

	std::vector<size_t> m_vertexCount;
	std::vector<uint32_t> m_indexCount;

	VkIndexType m_indexType;

	std::vector<std::vector<Vertex>> m_vertices;
	std::vector<std::vector<uint32_t>> m_indices;

	std::vector<Texture::GLTFMaterial> m_gltfMaterials;

	std::vector<int> m_gltfPrimitiveMaterialIndices;

	std::string m_optionalTexturePath = "";

	std::vector<VkDescriptorSet> m_descriptorSets;

	// | vertices of simple triangle
	const std::vector<Vertex> triangleVertices = {
	{{ 0.0f, -0.5f, 0.0f }, {1.0f, 0.0f, 0.0f}},
	{{ 0.5f,  0.5f, 0.0f }, {0.0f, 1.0f, 0.0f}},
	{{-0.5f,  0.5f, 0.0f }, {0.0f, 0.0f, 1.0f}}
	};

	std::vector<VkBuffer>& vertexBuffer() override {
		return m_vertexBuffer;
	};

	std::vector<VkDeviceMemory>& vertexMemory() override {
		return m_vertexMemory;
	}

	std::vector<VkBuffer>& indexBuffer() override {
		return m_indexBuffer;
	};

	std::vector<VkDeviceMemory>& indexMemory() override {
		return m_indexMemory;
	};

	std::vector<size_t>& vertexCount() override {
		return m_vertexCount;
	}

	std::vector<uint32_t>& indexCount() override {
		return m_indexCount;
	}

	VkIndexType& indexType() override {
		return m_indexType;
	}

	std::vector<std::vector<Vertex>>& vertices() override {
		return m_vertices;
	}

	std::vector<std::vector<uint32_t>>& indices() override {
		return m_indices;
	}

	std::vector<Texture::GLTFMaterial>& gltfMaterials() override {
		return m_gltfMaterials;
	}

	std::vector<int>& gltfPrimitiveMaterialIndices() override {
		return m_gltfPrimitiveMaterialIndices;
	}

	std::string& optionalTexturePath() override {
		return m_optionalTexturePath;
	}

	std::vector<VkDescriptorSet>& descriptorSets() override {
		return m_descriptorSets;
	}

};

