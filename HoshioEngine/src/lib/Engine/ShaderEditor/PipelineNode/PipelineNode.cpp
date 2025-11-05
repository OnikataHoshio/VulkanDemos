#include "Engine/ShaderEditor/PipelineNode/PipelineNode.h"

namespace HoshioEngine {
	TimestampQueries* PipelineNode::pTimestampQueries = nullptr;

	int PipelineNode::testCounter = 0;

	bool PipelineNode::isTesting = false;

	int PipelineNode::maxTestTimes = 1000;

	uint32_t PipelineNode::timestampCounter = 0;

	PipelineNode& PipelineNode::Init(){
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

	PipelineNode::PipelineNode()
	{
	}

	PipelineNode* PipelineNode::NextNode()
	{
		return next;
	}

	PipelineNode& PipelineNode::LinkNextNode(PipelineNode* node)
	{
		next = node;
		return *node;
	}

	void PipelineNode::Render()
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