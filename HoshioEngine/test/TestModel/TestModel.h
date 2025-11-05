#ifndef _TEST_MODEL_H_
#define _TEST_MODEL_H_

#include "Engine/ShaderEditor/PipelineNode/PipelineNode.h"
#include "Engine/Object/Model.h"

namespace HoshioEngine {
	class TestModel : public PipelineNode {
	private:
		Model model;
		ShaderInfo shader_info;
		int dsAttachments_id = M_INVALID_ID;
		int framebuffers_id = M_INVALID_ID;

		struct VertexUniform {
			glm::mat4 model = {};
			glm::mat4 view = {};
			glm::mat4 proj = {};
		}vertex_uniform;

		UniformBuffer vertex_uniform_buffer;
		UniformBuffer fragment_uniform_buffer;

		DescriptorSet uniform_set;

		void UpdateDescriptorSets() override;
		void RecordCommandBuffer() override;
		void InitResource() override;
		void CreateSampler() override;
		void CreateBuffer() override;
		void CreateRenderPass() override;
		void CreateDescriptorSetLayout() override;
		void CreatePipelineLayout() override;
		void CreatePipeline() override;
		void CreateFramebuffers() override;
		void OtherOperations() override;
		void SendDataToNextNode() override;
	public:
		TestModel(const char* file_path);
		void ImguiRender() override;

	};

}

#endif // !1
