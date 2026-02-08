#include "Callbacks.h"

void Callbacks::onKey(int key, int scancode, int action, int mods, RenderTarget& renderTarget, GLFWwindow* window) {

	auto& cameraEnabled = g_renderTarget.cameraEnabled;
	auto& renderTriangle = g_renderTarget.renderTriangle;
	auto& renderMandelbulb = g_renderTarget.renderMandelbulb;
	auto& rotationEnabled = g_renderTarget.rotationEnabled;

	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		cameraEnabled = !cameraEnabled;

		if (cameraEnabled)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	else if (key == GLFW_KEY_T && action == GLFW_PRESS) {
		if (renderTriangle)
			renderTriangle = false;
		else
			renderTriangle = true;
	}
	else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		renderMandelbulb = !renderMandelbulb;
	}
	else if (key == GLFW_KEY_G && action == GLFW_PRESS) {
		rotationEnabled = !rotationEnabled;
	}
}


void Callbacks::mouse_callback(GLFWwindow* window, double xpos, double ypos, Camera& camera) {

	auto& firstMouse = camera.firstMouse;
	auto& lastX = camera.lastX;
	auto& lastY = camera.lastY;

	if (firstMouse) {
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos; // reversed y

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// callback inside the class
void Callbacks::cursor_position_callback(GLFWwindow* window, double xpos, double ypos, Camera& camera) {

	auto& firstMouse = camera.firstMouse;
	auto& lastX = camera.lastX;
	auto& lastY = camera.lastY;

	if (firstMouse)
		lastX = xpos; lastY = ypos; firstMouse = false;

	float xoffset = float(xpos - lastX);
	float yoffset = float(lastY - ypos); // reversed: y ranges top->bottom
	lastX = xpos; lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}
