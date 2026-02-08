#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "RenderTarget.h"
#include "Camera.h"

class Callbacks
{

public:

	void onKey(int key, int scancode, int action, int mods, RenderTarget& renderTarget, GLFWwindow* window);

	void mouse_callback(GLFWwindow* window, double xpos, double ypos, Camera& camera);

	// callback inside the class
	void cursor_position_callback(GLFWwindow* window, double xpos, double ypos, Camera& camera);

};

