#pragma once
#include <glm/fwd.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ItemInterface.h"
#include "../Texture.h"
#include "../GPUTexture.h"
#include "../PhysXEngine.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>
#include <functional>
#include <cstdint>
#include <chrono>


class CozyHouse : public ItemInterface {

public:
	CozyHouse(VkDevice& device, VkPhysicalDevice& physicalDevice);


	void updatePC() override;

private:

	VkDevice& m_device;
	VkPhysicalDevice& m_physicalDevice;

};

