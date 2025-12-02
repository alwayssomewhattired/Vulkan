//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//
//#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
#include "MathConfig.hpp"
#include <cmath>
#include "Camera.h"
//#include <glm/detail/func_trigonometric.inl>
//#include <glm/detail/func_geometric.inl>

	Camera::Camera(glm::vec3 startPos, float yaw, float pitch, float speed, float sensitivity)
		: Position(startPos), Yaw(yaw), Pitch(pitch),
		MovementSpeed(speed), MouseSensitivity(sensitivity)
	{
	}

	glm::mat4 Camera::GetViewMatrix() const {
		return glm::lookAt(Position, Position + GetFront(), glm::vec3(0, 1, 0));
	}

	glm::vec3 Camera::GetFront() const {
		// convert yaw/pitch (degrees) to a direction vector
		float yawRad = glm::radians(Yaw);
		float pitchRad = glm::radians(Pitch);

		glm::vec3 front;
		front.x = cos(pitchRad) * cos(yawRad);
		front.y = sin(pitchRad);
		front.z = cos(pitchRad) * sin(yawRad);
		return glm::normalize(front);
	}

	glm::vec3 Camera::GetRight() const {
		return glm::normalize(glm::cross(GetFront(), glm::vec3(0, 1, 0)));
	}

	//glm::vec3 Camera::GetPosition() const {
	//	return glm::vec3(glm::inverse(view)[3]);
	//}

	void Camera::ProcessKeyboard(Camera_Movement dir, float dt) {
		float velocity = MovementSpeed * dt;

		if (dir == Camera_Movement::FORWARD) Position += GetFront() * velocity;
		if (dir == Camera_Movement::BACKWARD) Position -= GetFront() * velocity;
		if (dir == Camera_Movement::LEFT) Position -= GetRight() * velocity;
		if (dir == Camera_Movement::RIGHT) Position += GetRight() * velocity;
	}

	void Camera::ProcessMouseMovement(float xoff, float yoff, bool constrainPitch) {
		xoff *= MouseSensitivity;
		yoff *= MouseSensitivity;

		Yaw += xoff;
		Pitch += yoff;

		if (constrainPitch) {
			if (Pitch > 89.f) Pitch = 89.f;
			if (Pitch < -89.f) Pitch = -89.f;
		}
	}





