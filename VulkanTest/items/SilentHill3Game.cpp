#include "SilentHill3Game.h"
#include "../Constants.h"
#include "../Devices.h"
#include "../Camera.h"
#include <stdexcept>
#include <functional>
#include <cstdint>

SilentHill3Game::SilentHill3Game(VkDevice& device, VkPhysicalDevice& physicalDevice) : 
	m_device(device), m_physicalDevice(physicalDevice) {}

void SilentHill3Game::updatePC() {
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	glm::mat4& modelMatrixModel = modelMatrix.model;
	modelMatrixModel = glm::mat4(1.0f);

	modelMatrixModel = glm::translate(modelMatrixModel, glm::vec3(0.0f, 1.0f, 3.0f));

	// item view (tilted, rotating)
	modelMatrixModel = glm::rotate(modelMatrixModel,
	time * glm::radians(90.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));

	// | scaling
	modelMatrixModel = glm::scale(modelMatrixModel, glm::vec3(0.1f, 0.1f, 0.1f));
}