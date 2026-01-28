#pragma once
#include <glm/fwd.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ItemInterface.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <functional>
#include <cstdint>

class SilentHill3Game : public ItemInterface {
public:
	SilentHill3Game(VkDevice& device, VkPhysicalDevice& physicalDevice);

	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexMemory;

	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexMemory;

	size_t m_vertexCount;
	uint32_t m_indexCount;

	VkIndexType m_indexType;

	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;

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

	std::vector<VkDescriptorSet> m_descriptorSets;

	size_t m_modelUBOSize = sizeof(ModelUBO);

private:

	// | UBO holds model matrices
	// - use push-constants instead of ubo
	struct ModelUBO {
		alignas(16) glm::mat4 model;
	};
	VkDevice& m_device;
	VkPhysicalDevice& m_physicalDevice;


};