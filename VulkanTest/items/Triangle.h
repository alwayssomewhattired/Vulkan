
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
#include <chrono>

#include "ItemInterface.h"
#include "../Vertex.h"
#include "../glm_config.h"
#include "../Texture.h"
#include "../GPUTexture.h"
#include "../PhysXEngine.h"

class Triangle : public ItemInterface
{

public:

	// | vertices of simple triangle
	const std::vector<Vertex> triangleVertices = {
	{{ 0.0f, -0.5f, 0.0f }, {1.0f, 0.0f, 0.0f}},
	{{ 0.5f,  0.5f, 0.0f }, {0.0f, 1.0f, 0.0f}},
	{{-0.5f,  0.5f, 0.0f }, {0.0f, 0.0f, 1.0f}}
	};

	void updatePC() override;

};

