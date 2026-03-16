#pragma once


#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <chrono>

#include "ItemInterface.h"
#include "../Vertex.h"
#include "../glm_config.h"
#include "../Texture.h"
#include "../GPUTexture.h"
#include "../PhysXEngine.h"


class Home : public ItemInterface
{

public:

	void updatePC() override;

};

