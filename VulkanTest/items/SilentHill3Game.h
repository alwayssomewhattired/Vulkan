#pragma once
#include <glm/fwd.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <functional>
#include <cstdint>

class SilentHill3Game {

	SilentHill3Game(VkDevice& device);

	void createUniformBuffer(std::function<uint32_t(uint32_t typeFilter, VkMemoryPropertyFlags properties)>& findMemoryType);

	//void createModelDescriptorSets();

private:

	VkDevice m_device;

	struct ModelUBO {
		alignas(16) glm::mat4 model;
	};
};