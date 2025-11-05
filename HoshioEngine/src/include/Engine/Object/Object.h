#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "VulkanCommon.h"

namespace HoshioEngine {
	
	class Object {
	protected:
		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 rotation = glm::vec3(0.0f);
		glm::vec3 scale = glm::vec3(1.0f);

		glm::mat4x4 transform = glm::mat4x4(1.0f);

	public:
		Object(glm::vec3 position = glm::vec3(0.0f), glm::vec3 rotation = glm::vec3(0.0f),
			glm::vec3 scale = glm::vec3(1.0f));
		virtual ~Object() = default;

		//const 
		const glm::vec3& Position() const;

		const glm::vec3& Rotation() const;

		const glm::vec3& Scale() const;

		const glm::mat4x4& ModelTransform() const;

		//non-const
		void SetPosition(const glm::vec3& position);

		void SetRotation(const glm::vec3& rotation);

		void SetScale(const glm::vec3& scale);

	};

}

#endif