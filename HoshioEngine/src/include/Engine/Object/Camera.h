#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Object.h"
#include "Base\VulkanBase.h"

namespace HoshioEngine {
	enum class CAMERA_MOVEMENT {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN
	};


	class Camera : public Object {
	public:
		glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 cameraUp;
		glm::vec3 cameraRight;
		
		glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

		float zNear;

		float fovy;
		float cameraSpeed;
		float mouseSensitivity;

		Camera();
		virtual ~Camera() = default;

		glm::mat4x4 ViewTransform() const;
		
		glm::mat4x4 PerspectiveTransform() const;

		void ProcessKeyboard(CAMERA_MOVEMENT direction, float deltaTime);

		void ProcessMouseMovement(float xoffset, float yoffset);

		void ProcessMouseScroll(float yoffset);

	private:
		void UpdateCameraData();
	};
}



#endif // !1

