#include "StringLight.h"
#include "../Constants.h"
#include "../Devices.h"
#include "../Camera.h"
#include <stdexcept>
#include <functional>
#include <cstdint>

StringLight::StringLight(VkDevice& device, VkPhysicalDevice& physicalDevice) :
	m_device(device), m_physicalDevice(physicalDevice) {
}

void StringLight::m_updatePC() {
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	glm::mat4& modelMatrix = m_modelMatrix.model;
	modelMatrix = glm::mat4(1.0f);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 2.0f, 3.5f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	// | scaling
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.1f, 0.1f));

}