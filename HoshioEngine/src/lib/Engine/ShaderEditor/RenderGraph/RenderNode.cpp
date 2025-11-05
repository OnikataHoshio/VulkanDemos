#include "Engine/ShaderEditor/RenderGraph/RenderNode.h"

namespace HoshioEngine {
	TimestampQueries* RenderNode::pTimestampQueries = nullptr;

	int RenderNode::testCounter = 0;

	bool RenderNode::isTesting = false;

	int RenderNode::maxTestTimes = 1000;

	uint32_t RenderNode::timestampCounter = 0;

	RenderNode& RenderNode::Init(){
		InitResource();
		CreateSampler();
		CreateRenderPass();
		CreateDescriptorSetLayout();
		CreatePipelineLayout();
		CreatePipeline();
		CreateFramebuffers();
		CreateBuffer();
		OtherOperations();
		return *this;
	}

	RenderNode::RenderNode()
	{
	}

	RenderNode* RenderNode::NextNode()
	{
		return next;
	}

	RenderNode& RenderNode::LinkNextNode(RenderNode* node)
	{
		next = node;
		return *node;
	}

	void RenderNode::Render()
	{
		const CommandBuffer& commandBuffer = VulkanPlus::Plus().CommandBuffer_Graphics();
		if (isTesting) {
			if (isFirstNode) {
				pTimestampQueries->Reset(commandBuffer);
				timestampCounter = 0;
			}
			pTimestampQueries->WriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, timestampCounter++);
		}

		UpdateDescriptorSets();
		RecordCommandBuffer();

		if (isTesting) {
			pTimestampQueries->WriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, timestampCounter++);
		}

		if (next) {
			SendDataToNextNode();
			next->Render();
		}
	}
}