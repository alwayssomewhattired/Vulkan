#include "Home.h"

void Home::updatePC() {
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	glm::mat4& modelMatrixModel = modelMatrix.model;
	modelMatrixModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 4.0f));
}