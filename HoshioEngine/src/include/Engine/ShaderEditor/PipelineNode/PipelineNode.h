#ifndef _PIPELINE_NODE_H_
#define _PIPELINE_NODE_H_

#include "Base/CommandManager.h"
#include "Base/PipelineManager.h"
#include "Base/DescriptorManager.h"
#include "Base/RpwfManager.h"
#include "Base/SamplerManager.h"
#include "Base/QueryPoolManager.h"
#include "Plus/ImageManager.h"
#include "Plus/BufferManager.h"
#include "Utils/ImageUtils.h"

//imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

namespace HoshioEngine {

	class PipelineNode {
	protected:
		PipelineNode* next = nullptr;

		virtual void InitResource() = 0;
		virtual void CreateSampler() = 0;
		virtual void CreateBuffer() = 0;
		virtual void CreateRenderPass() = 0;
		virtual void CreateDescriptorSetLayout() = 0;
		virtual void CreatePipelineLayout() = 0;
		virtual void CreatePipeline() = 0;
		virtual void CreateFramebuffers() = 0;
		virtual void OtherOperations() = 0;
		virtual void UpdateDescriptorSets() = 0;
		virtual void RecordCommandBuffer() = 0;
		virtual void SendDataToNextNode() = 0;

	public:

		static TimestampQueries* pTimestampQueries;
		static uint32_t timestampCounter;
		static int testCounter;
		static int maxTestTimes;
		static bool isTesting;

		bool isFirstNode = false;

		PipelineNode();
		virtual ~PipelineNode() = default;
		virtual void ImguiRender() = 0;

		PipelineNode& Init();
		PipelineNode& LinkNextNode(PipelineNode* node);
		PipelineNode* NextNode();
		void Render();
	};
}


#endif // !_PIPELINE_NODE_H_

