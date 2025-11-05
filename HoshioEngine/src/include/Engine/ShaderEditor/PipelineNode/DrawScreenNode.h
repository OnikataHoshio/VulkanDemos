#ifndef _DRAW_SCREEN_NODE_H_
#define _DRAW_SCREEN_NODE_H_

#include "Engine/ShaderEditor/PipelineNode/PipelineNode.h"

namespace HoshioEngine {
	

	class DrawScreenNode : public PipelineNode {
	public:

	private:
		struct TexturePicker {
			ColorAttachment* colorAttachment = nullptr;
			Texture2D* texture = nullptr;
			enum class TextureMode {
				NO_MODE_SELECTED,
				COLOR_ATTACHMENT_MODE,
				TEXTURE_MODE
			}textureMode = TextureMode::NO_MODE_SELECTED;
		}texturePicker;

		struct PushConstant {
			int enableLod = 0;
			float lod = 0.0f;
		}pushConstant;

		VkSampler sampler = VK_NULL_HANDLE;

		std::vector<Pipeline> pipelines;
		PipelineLayout pipelineLayout;

		DescriptorSet descriptorSet;
		DescriptorSetLayout descriptorSetLayout;

		void SendDataToNextNode() override;
		void UpdateDescriptorSets() override;
		void RecordCommandBuffer() override;
		void CreateSampler() override;
		void CreateRenderPass() override;
		void CreateDescriptorSetLayout() override;
		void CreatePipelineLayout() override;
		void CreatePipeline() override;
		void CreateFramebuffers() override;
		void OtherOperations() override;
		void CreateBuffer() override;
		void InitResource() override;
	public:
		DrawScreenNode(VkSampler sampler);
		DrawScreenNode(VkSampler sampler, ColorAttachment* colorAttachment);
		DrawScreenNode(VkSampler sampler, Texture2D* texture);
		DrawScreenNode(VkSampler sampler, Texture2D& texture);

		uint32_t TextureMaxLevel() const;

		PushConstant& NodeParameter();

		void SetSampledImage(ColorAttachment* colorAttachment);

		void SetSampledImage(Texture2D* texture);

		// Í¨¹ý PipelineNode ¼Ì³Ð
		void ImguiRender() override;




	};
}


#endif // !_DRAW_SCREEN_NODE_H_
