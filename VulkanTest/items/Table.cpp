#include "Table.h"
#include "../Constants.h"
#include "../Devices.h"
#include "../Camera.h"
#include <stdexcept>
#include <functional>
#include <cstdint>

Table::Table(VkDevice& device, VkPhysicalDevice& physicalDevice) :
	m_device(device), m_physicalDevice(physicalDevice) {
}

void Table::m_updatePC(glm::mat4& modelMatrix, const bool rotationEnabled) {
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	modelMatrix = glm::mat4(1.0f);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(1.7f, 0.0f, 0.75f));

	modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	// | scaling
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.019f, 0.019f, 0.019f));
}