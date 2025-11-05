#ifndef _MESH_H_
#define _MESH_H_

#include "Plus/VulkanPlus.h"
#include "Engine/Object/Vertex.h"

namespace HoshioEngine {

	enum class TEXTURE_TYPE {
		DIFFUSE,
		SPECULAR,
		NORMAL,
		OTHER
	};

	struct TextureInfo {
		uint32_t id;
		TEXTURE_TYPE type;
	};

	struct ShaderInfo {
		int pipeline_id = M_INVALID_ID;
		int pipeline_layout_id = M_INVALID_ID;
		int renderpass_id = M_INVALID_ID;
		int sampler_id = M_INVALID_ID;
		int sampler_set_layout_id = M_INVALID_ID;
		int uniform_set_layout_id = M_INVALID_ID;
	};

	class Mesh {
	public:
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<TextureInfo> textures;

		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;

		DescriptorSet sampler_set;
		DescriptorSet uniform_set;

		Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<TextureInfo> textures);
		Mesh(const Mesh&) = delete;            
		Mesh& operator=(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = default;       
		Mesh& operator=(Mesh&&) noexcept = default;

		void Render(ShaderInfo& shader_info);
		void SetupMesh(ShaderInfo& shader_info);
		std::vector<float> ReorganizeVertexData();
		std::vector<VertexInputAttribute> GetVertexInputAttributes();
		uint32_t GetVertexInputAttributeStride();
	};



}


#endif // !_MESH_H_
