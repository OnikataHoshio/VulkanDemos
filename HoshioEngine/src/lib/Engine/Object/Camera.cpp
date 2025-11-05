#include "Engine/Object/Camera.h"

namespace HoshioEngine {
	Camera::Camera(): Object(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(-30.0f, -130.0f, 0.0f)),
		zNear(0.1f),fovy(45.0f),cameraSpeed(10.f),mouseSensitivity(0.05f)
	{
		UpdateCameraData();
	}

	glm::mat4x4 Camera::ViewTransform() const
	{
		return glm::lookAt(position, position + cameraFront, cameraUp);
	}

	glm::mat4x4 Camera::PerspectiveTransform() const
	{
		return FlipVertical(glm::infinitePerspectiveRH_ZO(glm::radians(fovy), VulkanBase::Base().AspectRatio(), zNear));
	}

	void Camera::ProcessKeyboard(CAMERA_MOVEMENT direction, float deltaTime)
	{
		float velocity = cameraSpeed * deltaTime;
		if (direction == CAMERA_MOVEMENT::FORWARD)
			position += velocity * cameraFront;
		if (direction == CAMERA_MOVEMENT::BACKWARD)
			position -= velocity * cameraFront;
		if (direction == CAMERA_MOVEMENT::RIGHT)
			position += velocity * cameraRight;
		if (direction == CAMERA_MOVEMENT::LEFT)
			position -= velocity * cameraRight;
		if (direction == CAMERA_MOVEMENT::UP)
			position += velocity * worldUp;
		if (direction == CAMERA_MOVEMENT::DOWN)
			position -= velocity * worldUp;
	}

	void Camera::ProcessMouseMovement(float xoffset, float yoffset)
	{
		xoffset *= mouseSensitivity;
		yoffset *= mouseSensitivity;

		rotation.y += xoffset;
		rotation.x += yoffset;
	
		if (rotation.x > 89.0f)
			rotation.x = 89.0f;
		if (rotation.x < -89.0f)
			rotation.x = 89.0f;

		UpdateCameraData();
	}

	void Camera::ProcessMouseScroll(float yoffset)
	{
		fovy -= (float)yoffset;
		if (fovy < 1.0f)
			fovy = 1.0f;
		if (fovy > 120.0f)
			fovy = 120.0f;

	}

	void Camera::UpdateCameraData()
	{
		float Yaw = rotation.y;
		float Pitch = rotation.x;
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		cameraFront = glm::normalize(front);

		cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
		cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
	}
}
