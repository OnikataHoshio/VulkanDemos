#include "Engine/Object/Mesh.h"
#include "Wins/GlfwManager.h"

namespace HoshioEngine {
	Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<TextureInfo> textures)
	{
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;
	}

	void Mesh::Render(ShaderInfo& shader_info)
	{
		//get the resources for rendering
		PipelineLayout& pipeline_layout = VulkanPlus::Plus().GetPipelineLayout(shader_info.pipeline_layout_id).second[0];

		//get the commandBuffer
		const CommandBuffer& commandBuffer = VulkanPlus::Plus().CommandBuffer_Graphics();

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffer.Address(), &offset);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		//bind sampler descriptor set
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 
			0, 1,sampler_set.Address(), 0, nullptr);
		vkCmdDrawIndexed(commandBuffer, indices.size(), 1, 0, 0, 0);
	}

	std::vector<float> Mesh::ReorganizeVertexData()
	{
		if (vertices.empty()) {
			return std::vector<float>();
		}

		size_t floats_per_vertex = vertices[0].GetVertexInputAttributeFloatCount();
		size_t total_float_count = floats_per_vertex * vertices.size();

		std::vector<float> vertex_data(total_float_count);

		for (size_t i = 0; i < vertices.size(); i++) {
			size_t vertex_start = i * floats_per_vertex;
			size_t float_offset = 0;

			memcpy(
				vertex_data.data() + vertex_start + float_offset,
				&vertices[i].position,
				3 * sizeof(float)
			);
			float_offset += 3;  

			memcpy(
				vertex_data.data() + vertex_start + float_offset,
				&vertices[i].normal,
				3 * sizeof(float)
			);
			float_offset += 3;  

			size_t color_count = vertices[i].GetColorCount();
			if (color_count != 0) {
				memcpy(
					vertex_data.data() + vertex_start + float_offset,
					vertices[i].colors.data(),
					color_count * 4 * sizeof(float)
				);
				float_offset += color_count * 4;
			}

			size_t uv_count = vertices[i].GetUVCount();
			if (uv_count != 0) {
				memcpy(
					vertex_data.data() + vertex_start + float_offset,
					vertices[i].uvs.data(),
					uv_count * 2 * sizeof(float)
				);
				float_offset += uv_count * 2;
			}

			size_t value_count = vertices[i].GetValueCount();
			if (value_count != 0) {
				memcpy(
					&vertex_data[vertex_start + float_offset],
					vertices[i].values.data(),
					value_count * sizeof(float)
				);
				float_offset += value_count;
			}
		}

		return vertex_data;
	}

	std::vector<VertexInputAttribute> Mesh::GetVertexInputAttributes()
	{
		if (vertices.empty()) {
			return std::vector<VertexInputAttribute>();
		}
		return vertices[0].GetVertexInputAttributes();
	}

	uint32_t Mesh::GetVertexInputAttributeStride()
	{
		if (vertices.empty()) {
			return 0;
		}
		return vertices[0].GetVertexInputAttributeStride();
	}

	void Mesh::SetupMesh(ShaderInfo& shader_info)
	{
		//create buffer for the mesh
		std::vector<float> reorganize_vertices = ReorganizeVertexData();
		vertexBuffer.Create(sizeof(float) * reorganize_vertices.size())
			.TransferData(reorganize_vertices.data(), sizeof(float) * reorganize_vertices.size());

		indexBuffer.Create(sizeof(uint32_t) * indices.size())
			.TransferData(indices.data(), sizeof(uint32_t) * indices.size());

		//allocate descriptor set for the mesh
		DescriptorSetLayout& sampler_set_layout = VulkanPlus::Plus().GetDescriptorSetLayout(shader_info.sampler_set_layout_id).second[0];
		DescriptorSetLayout& uniform_set_layout = VulkanPlus::Plus().GetDescriptorSetLayout(shader_info.uniform_set_layout_id).second[0];
		Sampler& sampler = VulkanPlus::Plus().GetSampler(shader_info.sampler_id).second[0];

		VulkanPlus::Plus().DescriptorPool().AllocateDescriptorSets(sampler_set, sampler_set_layout);
		VulkanPlus::Plus().DescriptorPool().AllocateDescriptorSets(uniform_set, uniform_set_layout);

		//setup sampler descriptor set for the mesh
		uint32_t count_diffuse = 0;
		uint32_t count_specular = 0;
		uint32_t count_normal = 0;
		uint32_t count_other = 0;
		for (auto& texture_info : textures) {
			Texture2D& texture = VulkanPlus::Plus().GetTexture2D(texture_info.id).second[0];
			switch (texture_info.type)
			{
			case TEXTURE_TYPE::DIFFUSE:
			{
				sampler_set.Write(texture.DescriptorImageInfo(sampler), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, count_diffuse++);
				break;
			}
			case TEXTURE_TYPE::SPECULAR:
				sampler_set.Write(texture.DescriptorImageInfo(sampler), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, count_specular++);
				break;
			case TEXTURE_TYPE::NORMAL:
				sampler_set.Write(texture.DescriptorImageInfo(sampler), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, count_normal++);
				break;
			default:
				sampler_set.Write(texture.DescriptorImageInfo(sampler), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, count_other++);
				break;
			}
		}

	}

}