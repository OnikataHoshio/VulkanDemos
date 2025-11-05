#include "Test3D.h"
#include "Wins/GlfwManager.h"

namespace HoshioEngine {

	void Test3D::UpdateDescriptorSets(){}

	void Test3D::RecordCommandBuffer()
	{
		struct PushConstant{
			glm::mat4 proj;
			glm::mat4 view;
		};
		static PushConstant pc;
		pc.proj = glm::infinitePerspectiveRH_ZO(glm::radians(60.0f),
			VulkanBase::Base().AspectRatio(),
			0.1f);
		static float theta = 0.0f;
		theta += static_cast<float>(GlfwWindow::deltaTime) * 100.0f;
		
		pc.view = GlfwWindow::camera.ViewTransform();
		pc.proj = GlfwWindow::camera.PerspectiveTransform();

		VkClearValue clearValues[2] = {
			{.color = { 0.f, 0.f, 0.f, 1.f } },
			{.depthStencil = { 1.f, 0 } }
		};

		const CommandBuffer& commandBuffer = VulkanPlus::Plus().CommandBuffer_Graphics();
		renderPass.Begin(commandBuffer,
			framebuffers[VulkanBase::Base().CurrentImageIndex()], {{},VulkanBase::Base().SwapchainCi().imageExtent},
			clearValues);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		VkBuffer buffers[2] = { vertexBuffer_perVertex, vertexBuffer_perInstance };
		VkDeviceSize offsets[2] = {};

		vkCmdBindVertexBuffers(commandBuffer, 0, 2, buffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, 128, &pc);
		vkCmdDrawIndexed(commandBuffer, 36, 12, 0, 0, 0);
		renderPass.End(commandBuffer);
	}

	void Test3D::ImguiRender()
	{
	}

	Test3D::Test3D(VkSampler sampler):sampler(sampler)
	{
	}

	void Test3D::InitResource()
	{
		GlfwWindow::is3d = true;
	}

	void Test3D::SendDataToNextNode()
	{
	}

	void Test3D::CreateSampler()
	{
	}

	void Test3D::CreateRenderPass()
	{
		//Create RenderPass
		{
			VkAttachmentDescription attachmentDescriptions[2] = {
				{
					.format = VulkanBase::Base().SwapchainCi().imageFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				},
				{
					.format = VK_FORMAT_D24_UNORM_S8_UINT,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				}
			};

			VkAttachmentReference attachmentReferences[2] = {
				{
					.attachment = 0,
					.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				},
				{
					.attachment = 1,
					.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				}
			};

			VkSubpassDescription subpassDescription = {
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments = attachmentReferences,
				.pDepthStencilAttachment = attachmentReferences + 1
			};

			VkSubpassDependency subpassDependency = {
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
			};

			VkRenderPassCreateInfo renderPassCreateInfo = {
				.attachmentCount = 2,
				.pAttachments = attachmentDescriptions,
				.subpassCount = 1,
				.pSubpasses = &subpassDescription,
				.dependencyCount = 1,
				.pDependencies = &subpassDependency
			};
			renderPass.Create(renderPassCreateInfo);
		}
	}

	void Test3D::CreateDescriptorSetLayout()
	{
	}

	void Test3D::CreatePipelineLayout()
	{
		//Create PipelineLayout
		{
			VkPushConstantRange pushConstantRange = { VK_SHADER_STAGE_VERTEX_BIT, 0, 128 };
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
				.pushConstantRangeCount = 1,
				.pPushConstantRanges = &pushConstantRange
			};
			pipelineLayout.Create(pipelineLayoutCreateInfo);
		}
	}

	void Test3D::CreatePipeline()
	{
		//Pipeline
		{
			auto Create = [&] {
				static ShaderModule vertModule("test/Test3D/Resource/shaders/SPIR-V/Test3D.vert.spv");
				static ShaderModule fragModule("test/Test3D/Resource/shaders/SPIR-V/Test3D.frag.spv");

				PipelineConfigurator configurator;
				configurator.PipelineLayout(pipelineLayout)
					.RenderPass(renderPass)
					.AddVertexInputBindings(0, sizeof(mVertex), VK_VERTEX_INPUT_RATE_VERTEX)
					.AddVertexInputBindings(1, sizeof(glm::vec3), VK_VERTEX_INPUT_RATE_INSTANCE)
					.AddVertexInputAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(mVertex, position))
					.AddVertexInputAttribute(1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(mVertex, color))
					.AddVertexInputAttribute(2, 1, VK_FORMAT_R32G32B32_SFLOAT, 0)
					.PrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
					.AddViewport(0.0f, 0.0f, static_cast<float>(VulkanBase::Base().SwapchainCi().imageExtent.width), static_cast<float>(VulkanBase::Base().SwapchainCi().imageExtent.height), 0.0f, 1.0f)
					.AddScissor(VkOffset2D{}, VulkanBase::Base().SwapchainCi().imageExtent)
					.CullMode(VK_CULL_MODE_BACK_BIT)
					.FrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
					.EnableDepthTest(VK_TRUE, VK_TRUE)
					.RasterizationSamples(VK_SAMPLE_COUNT_1_BIT)
					.AddAttachmentState(0b1111)
					.AddShaderStage(vertModule.ShaderStageCi(VK_SHADER_STAGE_VERTEX_BIT))
					.AddShaderStage(fragModule.ShaderStageCi(VK_SHADER_STAGE_FRAGMENT_BIT))
					.UpdatePipelineCreateInfo();
				pipeline.Create(configurator);
				};

			auto Destroy = [&] {
				pipeline.~Pipeline();
				};

			VulkanBase::Base().AddCallback_CreateSwapchain(Create);
			VulkanBase::Base().AddCallback_DestroySwapchain(Destroy);

			Create();
		}
	}

	void Test3D::CreateFramebuffers()
	{
		//Create Attachment 
		{
			auto Create = [&] {
				dsAttachments.resize(VulkanBase::Base().SwapchainImageCount());
				framebuffers.resize(VulkanBase::Base().SwapchainImageCount());
				for (auto& dsAttachment : dsAttachments)
					dsAttachment.Create(VK_FORMAT_D24_UNORM_S8_UINT, VulkanBase::Base().SwapchainExtent(), false, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);

				VkFramebufferCreateInfo framebufferCreateInfo = {
					.renderPass = renderPass,
					.attachmentCount = 2,
					.width = VulkanBase::Base().SwapchainExtent().width,
					.height = VulkanBase::Base().SwapchainExtent().height,
					.layers = 1
				};

				for (uint32_t i = 0; i < VulkanBase::Base().SwapchainImageCount(); i++) {
					VkImageView attachments[2] = {
						VulkanBase::Base().SwapchainImageView(i),
						dsAttachments[i].ImageView()
					};
					framebufferCreateInfo.pAttachments = attachments;
					framebuffers[i].Create(framebufferCreateInfo);
				}
				};

			auto Destroy = [&] {
				dsAttachments.clear();
				framebuffers.clear();
				};

			VulkanBase::Base().AddCallback_CreateSwapchain(Create);
			VulkanBase::Base().AddCallback_DestroySwapchain(Destroy);

			Create();
		}
	}

	void Test3D::OtherOperations()
	{
	}

	void Test3D::CreateBuffer()
	{
		//Create Buffer
		{
			mVertex vertices[] = {
				//x+
				{ {  1,  1, -1 }, { 1, 0, 0, 1 } },
				{ {  1, -1, -1 }, { 1, 0, 0, 1 } },
				{ {  1,  1,  1 }, { 1, 0, 0, 1 } },
				{ {  1, -1,  1 }, { 1, 0, 0, 1 } },
				//x-
				{ { -1,  1,  1 }, { 0, 1, 1, 1 } },
				{ { -1, -1,  1 }, { 0, 1, 1, 1 } },
				{ { -1,  1, -1 }, { 0, 1, 1, 1 } },
				{ { -1, -1, -1 }, { 0, 1, 1, 1 } },
				//y+
				{ {  1,  1, -1 }, { 0, 1, 0, 1 } },
				{ {  1,  1,  1 }, { 0, 1, 0, 1 } },
				{ { -1,  1, -1 }, { 0, 1, 0, 1 } },
				{ { -1,  1,  1 }, { 0, 1, 0, 1 } },
				//y-
				{ {  1, -1, -1 }, { 1, 0, 1, 1 } },
				{ { -1, -1, -1 }, { 1, 0, 1, 1 } },
				{ {  1, -1,  1 }, { 1, 0, 1, 1 } },
				{ { -1, -1,  1 }, { 1, 0, 1, 1 } },
				//z+
				{ {  1,  1,  1 }, { 0, 0, 1, 1 } },
				{ {  1, -1,  1 }, { 0, 0, 1, 1 } },
				{ { -1,  1,  1 }, { 0, 0, 1, 1 } },
				{ { -1, -1,  1 }, { 0, 0, 1, 1 } },
				//z-
				{ { -1,  1, -1 }, { 1, 1, 0, 1 } },
				{ { -1, -1, -1 }, { 1, 1, 0, 1 } },
				{ {  1,  1, -1 }, { 1, 1, 0, 1 } },
				{ {  1, -1, -1 }, { 1, 1, 0, 1 } }
			};

			glm::vec3 offsets[] = {
				{ -4, -4, -6 }, {  4, -4,  -6 },
				{ -4,  4, -10 }, {  4,  4, -10 },
				{ -4, -4, -14 }, {  4, -4, -14 },
				{ -4,  4, -18 }, {  4,  4, -18 },
				{ -4, -4, -22 }, {  4, -4, -22 },
				{ -4,  4, -26 }, {  4,  4, -26 }
			};


			vertexBuffer_perVertex.Create(sizeof vertices)
				.TransferData(vertices);

			vertexBuffer_perInstance.Create(sizeof offsets);
			vertexBuffer_perInstance.TransferData(offsets);

			uint16_t indices[36] = { 0, 2, 1, 2, 3, 1 };

			for (size_t i = 1; i < 6; i++)
				for (size_t j = 0; j < 6; j++)
					indices[i * 6 + j] = indices[j] + i * 4;

			indexBuffer.Create(sizeof indices);
			indexBuffer.TransferData(indices);
		}
	}

}