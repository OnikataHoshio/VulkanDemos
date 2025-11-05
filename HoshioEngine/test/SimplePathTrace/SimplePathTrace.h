#ifndef _SIMPLE_PATH_TRACE_H_
#define _SIMPLE_PATH_TRACE_H_

#include "Engine/ShaderEditor/RenderGraph/RenderNode.h"

namespace HoshioEngine {
	class SimplePathTrace : public RenderNode {
	private:
		Texture2D* texture = nullptr;

		VkSampler sampler = VK_NULL_HANDLE;

		Pipeline pipeline;
		PipelineLayout pipelineLayout;

		DescriptorSet descriptorSet;
		DescriptorSetLayout descriptorSetLayout;

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

		struct UniformBlock {
			uint32_t isOrthographic = 0;

			float kd = 1.0;
			float light_w = 0.5;
			float light_h = 0.5;
			float light_pos = -2.0;
			float light_offset_x = 0.0;
			float light_offset_y = 0.0;
			float light_power = 10.0;

			float view_pos = -10.0;

			float panel_pos = 1.0;

			uint32_t sample_count = 64;
		}u_Attribute;

		SimplePathTrace(VkSampler sampler, Texture2D* texture);


	};


}
#endif