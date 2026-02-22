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

	size_t m_modelUBOSize = sizeof(SilentHill3GamePC);

private:

	// - we need to actually make our shader use this push constants.
	// - currently we are using the same ubo as our 'home' model.
	// - this is causing our flickering of texture
	// - because we are racing the gpu.

	// | holds model matrices
	struct SilentHill3GamePC {
		alignas(16) glm::mat4 model;
	};

	VkDevice& m_device;
	VkPhysicalDevice& m_physicalDevice;


};