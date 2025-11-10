#ifndef _VERTEX_H_
#define _VERTEX_H_

#include "VulkanCommon.h"

namespace HoshioEngine {
	enum class VETEX_ATTRIBUTE_TYPE {
		POSITION,
		NORMAL,
		COLOR,
		UV,
		VALUE
	};

	struct VertexInputAttribute {
		uint32_t location;
		uint32_t binding;
		VkFormat format;
		uint32_t offset;
		VETEX_ATTRIBUTE_TYPE type;
	};


	class Vertex {
	private:
		template<typename T>
		void checkIndex(const std::vector<T>& container, std::string_view type, uint32_t& id) const {
			if (id >= container.size()) {
				std::cout << std::format("[ WARNING ] Vertex : {}_id ({}) is bigger than the count of {} ({})! Automatically clamped to max id ({})\n",
					type.data(), id, type.data(), container.size(), container.size() - 1);
				id = container.size() - 1;
			}
		}
	public:
		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 normal = glm::vec3(0.0f);
		std::vector<glm::vec4> colors;
		std::vector<glm::vec2> uvs;
		std::vector<float> values;

		Vertex(glm::vec3 position = glm::vec3(0.0f),
			glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f));

		Vertex(Vertex&& other) = default;
		Vertex& operator=(Vertex&& other) = default;

		Vertex(const Vertex& other) = default;
		Vertex& operator=(const Vertex& other) = default;

		virtual ~Vertex() = default;

		//const
		const glm::vec3& GetPosition() const;
		const glm::vec3& GetNormal() const;
		const glm::vec2& GetUV(uint32_t id = 0) const;
		const glm::vec4& GetColor(uint32_t id = 0) const;
		const float GetValue(uint32_t id = 0) const;

		//non-const
		void SetPosition(glm::vec3 position);
		void SetNormal(glm::vec3 normal);
		void SetUV(uint32_t id, glm::vec2 uv);
		void SetColor(uint32_t id, glm::vec4 color);
		void SetValue(uint32_t id, float value);

		void AddUV(glm::vec2 uv);
		void AddColor(glm::vec4 color);
		void AddValue(float value);

		
		size_t GetUVCount() const;
		size_t GetColorCount() const;
		size_t GetValueCount() const;

		uint32_t GetVertexInputAttributeStride();
		uint32_t GetVertexInputAttributeCount();
		uint32_t GetVertexInputAttributeFloatCount();
		std::vector<VertexInputAttribute> GetVertexInputAttributes();
	};
}

#endif // !_VERTEX_H_
