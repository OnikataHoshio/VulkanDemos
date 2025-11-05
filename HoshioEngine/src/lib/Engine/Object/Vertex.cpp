#include "Engine/Object/Vertex.h"

namespace HoshioEngine {
	Vertex::Vertex(glm::vec3 position, glm::vec3 normal)
		:position(position), normal(normal)
	{

	}

	const glm::vec3& Vertex::GetPosition() const {
		return position;
	}

	const glm::vec3& Vertex::GetNormal() const
	{
		return normal;
	}

	const glm::vec2& Vertex::GetUV(uint32_t id) const
	{
		checkIndex(uvs, "uv", id);
		return uvs[id];
	}

	const glm::vec4& Vertex::GetColor(uint32_t id) const
	{
		checkIndex(colors, "color", id);
		return colors[id];
	}

	const float Vertex::GetValue(uint32_t id) const
	{
		checkIndex(values, "value", id);
		return values[id];
	}


	void Vertex::SetPosition(glm::vec3 position)
	{
		this->position = position;
	}

	void Vertex::SetNormal(glm::vec3 normal)
	{
		this->normal = normal;
	}

	void Vertex::SetUV(uint32_t id, glm::vec2 uv)
	{
		checkIndex(uvs, "uv", id);
		uvs[id] = uv;
	}

	void Vertex::SetColor(uint32_t id, glm::vec4 color)
	{
		checkIndex(colors, "color", id);
		colors[id] = color;
	}

	void Vertex::SetValue(uint32_t id, float value)
	{
		checkIndex(values, "value", id);
		values[id] = value;
	}

	void Vertex::AddUV(glm::vec2 uv)
	{
		uvs.push_back(uv);
	}

	void Vertex::AddColor(glm::vec4 color)
	{
		colors.push_back(color);
	}

	void Vertex::AddValue(float value)
	{
		values.push_back(value);
	}

	size_t Vertex::GetUVCount() const
	{
		return uvs.size();
	}

	size_t Vertex::GetColorCount() const
	{
		return colors.size();
	}

	size_t Vertex::GetValueCount() const
	{
		return values.size();
	}

	uint32_t Vertex::GetVertexInputAttributeStride()
	{
		//cal the stride of the group attribute of a vertex (Byte)
		return (3 + 3 + 4 * colors.size() + 2 * uvs.size() + values.size()) * sizeof(float);
	}

	uint32_t Vertex::GetVertexInputAttributeCount()
	{
		return 2 + colors.size() + uvs.size() + values.size();
	}

	uint32_t Vertex::GetVertexInputAttributeFloatCount()
	{
		return 3 + 3 + 4 * colors.size() + 2 * uvs.size() + values.size();
	}

	std::vector<VertexInputAttribute> Vertex::GetVertexInputAttributes()
	{
		std::vector<VertexInputAttribute> vertex_inpute_attributes;

		uint32_t location = 0;
		uint32_t offset = 0;

		//position
		VertexInputAttribute attribute = {
			.location = location,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offset,
			.type = VETEX_ATTRIBUTE_TYPE::POSITION
			
		};
		vertex_inpute_attributes.push_back(attribute);
		location += 1;
		offset += 3 * sizeof(float);

		//normal
		attribute.location = location;
		attribute.offset = offset;
		attribute.type =  VETEX_ATTRIBUTE_TYPE::NORMAL;
		vertex_inpute_attributes.push_back(attribute);
		location += 1;
		offset += 3 * sizeof(float);

		//color
		attribute.type = VETEX_ATTRIBUTE_TYPE::COLOR;
		for (size_t i = 0; i < colors.size(); i++) {
			attribute.location = location;
			attribute.offset = offset;
			attribute.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			vertex_inpute_attributes.push_back(attribute);
			location += 1;
			offset += 4 * sizeof(float);
		}

		//uvs
		attribute.type = VETEX_ATTRIBUTE_TYPE::UV;
		for (size_t i = 0; i < uvs.size(); i++) {
			attribute.location = location;
			attribute.offset = offset;
			attribute.format = VK_FORMAT_R32G32_SFLOAT;
			vertex_inpute_attributes.push_back(attribute);
			location += 1;
			offset += 2 * sizeof(float);
		}

		//values
		attribute.type = VETEX_ATTRIBUTE_TYPE::VALUE;
		for (size_t i = 0; i < values.size(); i++) {
			attribute.location = location;
			attribute.offset = offset;
			attribute.format = VK_FORMAT_R32_SFLOAT;
			vertex_inpute_attributes.push_back(attribute);
			location += 1;
			offset += sizeof(float);
		}
		
		return vertex_inpute_attributes;
	}
}
