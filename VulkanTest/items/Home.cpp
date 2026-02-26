#include "Home.h"

void Home::m_updatePC(glm::mat4& modelMatrix, const bool rotationEnabled) {
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	// item view (tilted, rotating)
	if (rotationEnabled) {
		modelMatrix = glm::rotate(glm::mat4(1.0f),
			time * glm::radians(90.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else {
		// normal view (straight, unchanging)
		modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 4.0f));
	}
}