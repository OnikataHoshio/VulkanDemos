#ifndef _TEST_3D_H_
#define _TEST_3D_H_

#include "Engine/ShaderEditor/RenderGraph/RenderNode.h"
#include "Engine/Actor/Camera.h"

namespace HoshioEngine {
	class Test3D : public RenderNode{
	private:
		struct mVertex {
			glm::vec3 position;
			glm::vec4 color;
		};

		RenderPass renderPass;
		std::vector<DepthStencilAttachment> dsAttachments;
		std::vector<Framebuffer> framebuffers;

		VertexBuffer vertexBuffer_perVertex;
		VertexBuffer vertexBuffer_perInstance;
		IndexBuffer indexBuffer;

		VkSampler sampler = VK_NULL_HANDLE;

		Pipeline pipeline;
		PipelineLayout pipelineLayout;

		void InitResource() override;
		void UpdateDescriptorSets() override;
		void RecordCommandBuffer() override;
		void SendDataToNextNode() override;
		void CreateSampler() override;
		void CreateRenderPass() override;
		void CreateDescriptorSetLayout() override;
		void CreatePipelineLayout() override;
		void CreatePipeline() override;
		void CreateFramebuffers() override;
		void OtherOperations() override;
		void CreateBuffer() override;
	public:
		void ImguiRender() override;
		Test3D(VkSampler sampler);
	};
}

#endif // !_TEST_3D_H_

