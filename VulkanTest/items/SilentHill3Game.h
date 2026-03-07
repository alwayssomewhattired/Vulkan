#pragma once
#include <glm/fwd.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ItemInterface.h"
#include "../Texture.h"
#include "../GPUTexture.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <functional>
#include <cstdint>
#include <chrono>


class SilentHill3Game : public ItemInterface {
public:
	SilentHill3Game(VkDevice& device, VkPhysicalDevice& physicalDevice);

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

	// | holds model matrices
	ModelMatrix m_modelMatrix;

	struct alignas(16) MaterialUBO {
		glm::vec4 baseColorFactor;
	};

	//std::vector<MaterialUBO> materialUniformBuffers;

	// | contiguous (material index + frame)
	std::vector<VkBuffer> m_materialUniformBuffers;
	std::vector<VkDeviceMemory> m_materialUniformBuffersMemory;
	std::vector<void*> m_materialUniformBuffersMapped;

	void m_updatePC(glm::mat4& modelMatrix, const bool rotationEnabled);

	// | begin item interface block
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

	ModelMatrix& modelMatrix() override {
		return m_modelMatrix;
	}

	void updatePC(glm::mat4& modelMatrix, const bool rotationEnabled) override {
		m_updatePC(modelMatrix, rotationEnabled);
	}

	std::vector<VkBuffer>& materialUniformBuffers() override {
		return m_materialUniformBuffers;
	}

	std::vector<VkDeviceMemory>& materialUniformBuffersMemory() override {
		return m_materialUniformBuffersMemory;
	}

	std::vector<void*>& materialUniformBuffersMapped() override {
		return m_materialUniformBuffersMapped;
	}

	//size_t m_modelUBOSize = sizeof(m_modelMatrix);

private:

	// - we need to actually make our shader use this push constants.

	VkDevice& m_device;
	VkPhysicalDevice& m_physicalDevice;

};