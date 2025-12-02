#pragma once

#include "MathConfig.hpp"
#include <cmath>

enum class Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

class Camera {
public:
	glm::vec3 Position;
	float Yaw;
	float Pitch;
	float MovementSpeed;
	float MouseSensitivity;

	Camera(glm::vec3 startPos = { 0.0f, 1.6f, 5.0f },
		float yaw = -90.f, float pitch = 0.0f,
		float speed = 6.0f, float sensitivity = 0.1);

	glm::mat4 GetViewMatrix() const;

	glm::vec3 GetFront() const;

	glm::vec3 GetRight() const;

	//glm::vec3 GetPosition() const;

	void ProcessKeyboard(Camera_Movement dir, float deltaTime);

	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

	struct CameraUBO {
		glm::mat4 view;
		glm::mat4 proj;
	};
};