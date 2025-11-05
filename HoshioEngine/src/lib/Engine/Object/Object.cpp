#include "Engine/Object/Object.h"

namespace HoshioEngine {
	Object::Object(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale)
		:position(position), rotation(rotation), scale(scale)
	{
	}

	const glm::vec3& Object::Position() const
	{
		return position;
	}

	const glm::vec3& Object::Rotation() const
	{
		return rotation;
	}

	const glm::vec3& Object::Scale() const
	{
		return scale;
	}

	const glm::mat4x4& Object::ModelTransform() const
	{
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
		glm::mat4 rotationMatrix = 
			glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) * 
			glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) * 
			glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));  
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
		return translationMatrix * rotationMatrix * scaleMatrix;
	}

	void Object::SetPosition(const glm::vec3& position)
	{
		this->position = position;
	}

	void Object::SetRotation(const glm::vec3& rotation)
	{
		this->rotation = rotation;
	}

	void Object::SetScale(const glm::vec3& scale)
	{
		this->scale = scale;
	}

}

