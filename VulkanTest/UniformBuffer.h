#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <chrono>
#include "glm_config.h"
#include <cstdint>
#include <vector>

#include "Camera.h"
#include "Constants.h"
#include "Devices.h"
#include "SwapChain.h"
#include "items/ItemInterface.h"

struct ItemInterface;

class UniformBuffer
{
public:
	UniformBuffer(Devices& devices, Camera& camera, SwapChain& swapChain, const bool& rotationEnabled);

	void createUniformBuffers();

	void createMaterialUniformBuffer(ItemInterface& item);

	void updateCameraUniformBuffer(uint32_t currentImage);

	void createAnimationUniformBuffer(ItemInterface& item);

	void updateMandelbulbUBO(uint32_t currentImage);

	void updateAnimationUBO(ItemInterface& item, uint32_t currentFrame);

	uint32_t maxPCSize;
	uint32_t cameraUBOSize;

	struct MandelbulbUBO {

		glm::mat4 invProjection;
		glm::mat4 invView;

		glm::vec4 camPos_time;       // xyz = cameraPos, w = time
		glm::vec4 resolution_misc;   // xy = resolution, z = power, w = bail

		alignas(16) int maxIter;
		glm::vec3 pad;               // explicit padding
	};

	// | Camera UBOS
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	std::vector<VkBuffer> mandelbulbUniformBuffers;
	std::vector<VkDeviceMemory> mandelbulbUniformBuffersMemory;
	std::vector<void*> mandelbulbUniformBuffersMapped;

private:
	Devices& m_Devices;
	Camera& m_Camera;
	SwapChain& m_SwapChain;
	const bool& m_rotationEnabled;

};

