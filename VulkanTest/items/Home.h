#pragma once
#include <glm/fwd.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "ItemInterface.h"
#include <cstdint>

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
	
	size_t vertexCount() override {
		return m_vertexCount;
	}

	uint32_t& indexCount() override {
		return m_indexCount;
	}
		
	VkIndexType indexType() override {
		return m_indexType;
	}
};

