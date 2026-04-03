#include "CozyHouse.h"
#include "../Constants.h"
#include "../Devices.h"
#include "../Camera.h"
#include <stdexcept>
#include <functional>
#include <cstdint>

CozyHouse::CozyHouse(VkDevice& device, VkPhysicalDevice& physicalDevice) :
	m_device(device), m_physicalDevice(physicalDevice) {
}

void CozyHouse::updatePC() {
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	glm::mat4& modelMatrixModel = modelMatrix.model;
	modelMatrixModel = glm::mat4(1.0f);

	modelMatrixModel = glm::translate(modelMatrixModel, glm::vec3(-4.0f, 0.0f, 0.75f));

	modelMatrixModel = glm::rotate(modelMatrixModel, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	// | scaling
	modelMatrixModel = glm::scale(modelMatrixModel, glm::vec3(0.5f, 0.5f, 0.5f));
}