#pragma once
#include <glm/fwd.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>

class SilentHill3Game {

	void createUniformBuffer() {

	}

	void createModelDescriptorSets() {

	}

private:

	struct ModelUBO {
		alignas(16) glm::mat4 model;
	};
};