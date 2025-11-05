#define STB_IMAGE_IMPLEMENTATION
#include "Wins/GlfwManager.h"

#include "Engine/ShaderEditor/RenderGraph/DrawScreenNode.h"
#include "Engine/Panel/Editor/EditorGUIManager.h"
#include "Engine/Panel/Editor/EditorInspectorPanel.h"
#include "test/SimplePathTrace/SimplePathTrace.h"
#include "test/Test3D/Test3D.h"
#include "test/TestModel/TestModel.h"
#include "test/TestCurve/CurvePanel.h"

using namespace HoshioEngine;

int main() {
	{
		if (!GlfwWindow::InitializeWindow({1840, 1024 }))
			return -1;

		Fence& fence = VulkanPlus::Plus().CreateFences("test-fence", 1).second[0];

		Semaphore semaphore_image_available;
		Semaphore semaphore_render_over;

		VulkanPlus::Plus().CreateTexture2D("test", "res/images/icon-1024.png", VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, true);
		Texture2D& texture = VulkanPlus::Plus().GetTexture2D("test").second[0];
		Texture2D bg("res/images/kayoko-bg.png", VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, true);


		Sampler& sampler_linear = VulkanPlus::Plus().CreateSampler("sampler_linear", Sampler::SamplerCreateInfo()).second[0];

		TimestampQueries timestampQueries(7);
		RenderNode::pTimestampQueries = &timestampQueries;
		
		const CommandBuffer& commandBuffer = VulkanPlus::Plus().CommandBuffer_Graphics();

		{
			std::unique_ptr<RenderNode> drawScreenNode =
				std::make_unique<DrawScreenNode>(sampler_linear, bg);
			drawScreenNode->Init();

			//std::unique_ptr<RenderNode> simplePathTrace =
			//	std::make_unique<SimplePathTrace>(sampler_linear, &texture);
			//simplePathTrace->Init();

			std::unique_ptr<RenderNode> test3D =
				std::make_unique<Test3D>(sampler_linear);
			test3D->Init();

			//std::unique_ptr<RenderNode> test_model =
			//	std::make_unique<TestModel>("test/TestModel/Resource/Models/backpack/backpack.obj");
			//test_model->Init();

			EditorGUIManager::Instance().editorPanels.push_back(std::make_unique<CurvePanel>());

			while (!glfwWindowShouldClose(GlfwWindow::pWindow)) {
				while (glfwGetWindowAttrib(GlfwWindow::pWindow, GLFW_ICONIFIED))
					glfwWaitEvents();

				VulkanBase::Base().SwapImage(semaphore_image_available);

				auto i = VulkanBase::Base().CurrentImageIndex();

				commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

				drawScreenNode->Render();

				commandBuffer.End();

				EditorGUIManager::Instance().Render();

				VkCommandBuffer commandBuffers[] = { commandBuffer, EditorGUIManager::Instance().GCommandBuffer() };
				VulkanBase::Base().SubmitCommandBuffer_Graphics(commandBuffers, semaphore_image_available, semaphore_render_over, fence);
				VulkanBase::Base().PresentImage(semaphore_render_over);

				glfwPollEvents();
				GlfwWindow::UpdateWindow();

				fence.Wait();
				fence.Reset();
			}

			VulkanBase::Base().WaitIdle();
		}
		VulkanBase::Base().WaitIdle();

	}
	return 0;
}




