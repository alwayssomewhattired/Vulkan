#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <chrono>
#include "glm_config.h"

#include "Camera.h"
#include "Constants.h"
#include "Devices.h"
#include "SwapChain.h"

class UniformBuffer
{
public:
	UniformBuffer(Devices& devices, Camera& camera, SwapChain& swapChain, const bool& rotationEnabled);

	void createUniformBuffers();

	void createUniformBuffer(const size_t& UBOSize);

	void updateUniformBuffer(uint32_t currentImage);

	void updateMandelbulbUBO(uint32_t currentImage);

	void updateModelBuffer(uint32_t currentImage);

	// | dynamic ubo
	struct ModelUBO {
		alignas(16) glm::mat4 model;
	};

	uint32_t alignedModelUBOSize;

	struct MandelbulbUBO {

		glm::mat4 invProjection;
		glm::mat4 invView;

		glm::vec4 camPos_time;       // xyz = cameraPos, w = time
		glm::vec4 resolution_misc;   // xy = resolution, z = power, w = bail

		alignas(16) int maxIter;
		glm::vec3 pad;               // explicit padding
	};

	// | I think this is the uniform buffers for the triangle
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	std::vector<VkBuffer> modelUniformBuffers;
	std::vector<VkDeviceMemory> modelUniformBuffersMemory;
	std::vector<void*> modelUniformBuffersMapped;

	std::vector<VkBuffer> mandelbulbUniformBuffers;
	std::vector<VkDeviceMemory> mandelbulbUniformBuffersMemory;
	std::vector<void*> mandelbulbUniformBuffersMapped;

private:
	Devices& m_Devices;
	Camera& m_Camera;
	SwapChain& m_SwapChain;
	const bool& m_rotationEnabled;

};

