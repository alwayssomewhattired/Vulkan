
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

	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexMemory;

	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexMemory;

	size_t m_vertexCount;
	uint32_t m_indexCount;

	VkIndexType m_indexType;

	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;

	std::vector<Texture::GPUMaterial> m_gpuMaterials;

	std::string m_optionalTexturePath = "";

	std::vector<VkDescriptorSet> m_descriptorSets;

	// | vertices of simple triangle
	const std::vector<Vertex> triangleVertices = {
	{{ 0.0f, -0.5f, 0.0f }, {1.0f, 0.0f, 0.0f}},
	{{ 0.5f,  0.5f, 0.0f }, {0.0f, 1.0f, 0.0f}},
	{{-0.5f,  0.5f, 0.0f }, {0.0f, 0.0f, 1.0f}}
	};

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

	std::vector<Texture::GPUMaterial>& gpuMaterials() override {
		return m_gpuMaterials;
	}

	std::string& optionalTexturePath() override {
		return m_optionalTexturePath;
	}

	std::vector<VkDescriptorSet>& descriptorSets() override {
		return m_descriptorSets;
	}

};

