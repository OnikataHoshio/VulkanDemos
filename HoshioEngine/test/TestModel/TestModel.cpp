#include "TestModel.h"
#include "Wins/GlfwManager.h"

namespace HoshioEngine {
	TestModel::TestModel(const char* file_path) : model(file_path)
	{
	}

	void TestModel::UpdateDescriptorSets()
	{
		vertex_uniform.model = glm::mat4(1.0f);
		vertex_uniform.view = GlfwWindow::camera.ViewTransform();
		vertex_uniform.proj = GlfwWindow::camera.PerspectiveTransform();

		vertex_uniform_buffer.TransferData(&vertex_uniform, sizeof vertex_uniform);

		VkDescriptorBufferInfo bufferInfo = {
			.buffer = vertex_uniform_buffer,
			.offset = 0,
			.range = sizeof vertex_uniform
		};

		uniform_set.Write(bufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
		//bufferInfo.buffer = fragment_uniform_buffer;
		//bufferInfo.offset = 0;
		//bufferInfo.range = 4;
		//uniform_set.Write(bufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
	}

	void TestModel::RecordCommandBuffer()
	{
		uint32_t current_image_ind = VulkanBase::Base().CurrentImageIndex();

		RenderPass& renderPass = VulkanPlus::Plus().GetRenderPass(shader_info.renderpass_id).second[0];
		const CommandBuffer& commandBuffer = VulkanPlus::Plus().CommandBuffer_Graphics();
		std::span<Framebuffer> framebuffers = VulkanPlus::Plus().GetFramebuffers(framebuffers_id).second;

		VkRect2D renderArea = {{}, VulkanBase::Base().SwapchainExtent() };
		VkClearValue clearValues[2] = {
			{.color = { 0.f, 0.f, 0.f, 1.f } },
			{.depthStencil = { 1.f, 0 } }
		};

		renderPass.Begin(commandBuffer, framebuffers[current_image_ind], renderArea, clearValues);
		PipelineLayout& pipeline_layout = VulkanPlus::Plus().GetPipelineLayout(shader_info.pipeline_layout_id).second[0];
		DescriptorSetLayout& uniform_set_layout = VulkanPlus::Plus().GetDescriptorSetLayout(shader_info.uniform_set_layout_id).second[0];
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout,
			1, 1, uniform_set.Address(), 0, nullptr);
		model.Render(shader_info);
		renderPass.End(commandBuffer);
	}

	void TestModel::InitResource()
	{
	}

	void TestModel::CreateSampler()
	{
		//create sampler
		shader_info.sampler_id = VulkanPlus::Plus().CreateSampler("trilinear-sampler", Sampler::SamplerCreateInfo()).first;
	}
	void TestModel::CreateBuffer()
	{
		
		vertex_uniform_buffer.Create(sizeof vertex_uniform);
			

		//fragment_uniform_buffer.Create(4);
	}

	void TestModel::CreateRenderPass()
	{
		//create renderpass
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
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				}
			};

			VkAttachmentReference attachmentReferences[2] = {
				{
					.attachment = 0,
					.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
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

			shader_info.renderpass_id = VulkanPlus::Plus().CreateRenderPass("swapchain-size-depth-renderpass", renderPassCreateInfo).first;
		}
	}
	void TestModel::CreateDescriptorSetLayout()
	{
		//create sampler set layout
		{
			VkDescriptorSetLayoutBinding bindings[4] = {
				//DIFFUSE
				{
					.binding = 0,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 4,
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
				},
				//SPECULAR
				{
					.binding = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 4,
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
				},
				//NORMAL
				{
					.binding = 2,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 2,
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
				},
				//OTHER
				{
					.binding = 3,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 4,
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
				}
			};

			VkDescriptorBindingFlags flags[4]{};
			flags[0] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
			flags[1] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
			flags[2] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
			flags[3] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

			VkDescriptorSetLayoutBindingFlagsCreateInfo flags_ci{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
				.bindingCount = 4,
				.pBindingFlags = flags
			};


			VkDescriptorSetLayoutCreateInfo createInfo = {
				.pNext = &flags_ci,
				.bindingCount = 4,
				.pBindings = bindings
			};
			shader_info.sampler_set_layout_id = VulkanPlus::Plus().CreateDescriptorSetLayout("model-sampler-set-layout", createInfo).first;
		}

		//create uniform set layout
		{
			VkDescriptorSetLayoutBinding bindings[2] = {
				{
					.binding = 0,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.descriptorCount = 1,
					.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
				},
				{
					.binding = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.descriptorCount = 1,
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
				}
			};
			VkDescriptorSetLayoutCreateInfo createInfo = {
				.bindingCount = 2,
				.pBindings = bindings
			};
			shader_info.uniform_set_layout_id = VulkanPlus::Plus().CreateDescriptorSetLayout("model-uniform-set-layot", createInfo).first;
			DescriptorSetLayout& uniform_set_layout = VulkanPlus::Plus().GetDescriptorSetLayout(shader_info.uniform_set_layout_id).second[0];
			VulkanPlus::Plus().DescriptorPool().AllocateDescriptorSets(uniform_set, uniform_set_layout);
		}
	}

	void TestModel::CreatePipelineLayout()
	{
		DescriptorSetLayout& sampler_set_layout = VulkanPlus::Plus().GetDescriptorSetLayout(shader_info.sampler_set_layout_id).second[0];
		DescriptorSetLayout& uniform_set_layout = VulkanPlus::Plus().GetDescriptorSetLayout(shader_info.uniform_set_layout_id).second[0];
		VkDescriptorSetLayout layouts[] = { sampler_set_layout, uniform_set_layout };

		VkPipelineLayoutCreateInfo createInfo = {
			.setLayoutCount = 2,
			.pSetLayouts = layouts
		};

		shader_info.pipeline_layout_id = VulkanPlus::Plus().CreatePipelineLayout("model-pipeline-layout", createInfo).first;

	}

	void TestModel::CreatePipeline()
	{
		auto Create = [&] {
			static ShaderModule vertModule("test/TestModel/Resource/Shaders/SPIR-V/Blinn-Phong.vert.spv");
			static ShaderModule fragModule("test/TestModel/Resource/Shaders/SPIR-V/Blinn-Phong.frag.spv");

			PipelineConfigurator configurator;
			std::vector<VertexInputAttribute> vertex_input_attributes = model.GetVertexInputeAttributes();
			uint32_t stride = model.GetVertexInputAttributesStride();
			configurator.AddVertexInputBindings(0, stride, VK_VERTEX_INPUT_RATE_VERTEX);
			for (auto& attribute : vertex_input_attributes) {
				configurator.AddVertexInputAttribute(
					attribute.location, attribute.binding, attribute.format, attribute.offset);
			}

			PipelineLayout& pipeline_layout = VulkanPlus::Plus().GetPipelineLayout(shader_info.pipeline_layout_id).second[0];
			RenderPass& renderpass = VulkanPlus::Plus().GetRenderPass(shader_info.renderpass_id).second[0];
			configurator.PipelineLayout(pipeline_layout)
				.RenderPass(renderpass)
				.PrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
				.AddViewport(0.0f, 0.0f, 
					static_cast<float>(VulkanBase::Base().SwapchainCi().imageExtent.width), 
					static_cast<float>(VulkanBase::Base().SwapchainCi().imageExtent.height),
					0.0f, 1.0f)
				.AddScissor(VkOffset2D{}, VulkanBase::Base().SwapchainCi().imageExtent)
				.CullMode(VK_CULL_MODE_BACK_BIT)
				.FrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE)
				.EnableDepthTest(VK_TRUE, VK_TRUE)
				.RasterizationSamples(VK_SAMPLE_COUNT_1_BIT)
				.AddAttachmentState(0b1111)
				.AddShaderStage(vertModule.ShaderStageCi(VK_SHADER_STAGE_VERTEX_BIT))
				.AddShaderStage(fragModule.ShaderStageCi(VK_SHADER_STAGE_FRAGMENT_BIT))
				.UpdatePipelineCreateInfo();
			shader_info.pipeline_id = VulkanPlus::Plus().CreatePipeline("test-model-pipeline",configurator).first;
			};

		auto Destroy = [&] {
			VulkanPlus::Plus().DestroyPipeline(shader_info.pipeline_id);
			};

		VulkanBase::Base().AddCallback_CreateSwapchain(Create);
		VulkanBase::Base().AddCallback_DestroySwapchain(Destroy);

		Create();
	}

	void TestModel::CreateFramebuffers()
	{
		auto Create = [&] {
			RenderPass& renderpass = VulkanPlus::Plus().GetRenderPass(shader_info.renderpass_id).second[0];
			dsAttachments_id = VulkanPlus::Plus().CreateDepthStencilAttachments(
				"testmodel-ds-attachments",
				VulkanBase::Base().SwapchainImageCount(),
				VK_FORMAT_D24_UNORM_S8_UINT,
				VulkanBase::Base().SwapchainExtent(),
				false, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT).first;
			std::span<DepthStencilAttachment> dsAttachments = VulkanPlus::Plus().GetDepthStencilAttachments(dsAttachments_id).second;
			VkFramebufferCreateInfo framebufferCreateInfo = {
				.renderPass = renderpass,
				.attachmentCount = 2,
				.width = VulkanBase::Base().SwapchainExtent().width,
				.height = VulkanBase::Base().SwapchainExtent().height,
				.layers = 1
			};
			std::vector<VkFramebufferCreateInfo> framebufferCreateInfos(VulkanBase::Base().SwapchainImageCount());
			VkImageView attachments[3][2];
			for (uint32_t i = 0; i < VulkanBase::Base().SwapchainImageCount(); i++) {
				framebufferCreateInfos[i].renderPass = renderpass;
				framebufferCreateInfos[i].attachmentCount = 2;
				framebufferCreateInfos[i].width = VulkanBase::Base().SwapchainExtent().width;
				framebufferCreateInfos[i].height = VulkanBase::Base().SwapchainExtent().height;
				framebufferCreateInfos[i].layers = 1;
				attachments[i][0] = VulkanBase::Base().SwapchainImageView(i);
				attachments[i][1] = dsAttachments[i].ImageView();
				framebufferCreateInfos[i].pAttachments = attachments[i];
			}
			framebuffers_id = VulkanPlus::Plus().CreateFramebuffers("testmodel-framebuffers", VulkanBase::Base().SwapchainImageCount(), framebufferCreateInfos).first;
			};

		auto Destroy = [&] {
			VulkanPlus::Plus().DestroyDepthStencilAttachments(dsAttachments_id);
			VulkanPlus::Plus().DestroyFramebuffers(framebuffers_id);
			};

		VulkanBase::Base().AddCallback_CreateSwapchain(Create);
		VulkanBase::Base().AddCallback_DestroySwapchain(Destroy);

		Create();
	}

	void TestModel::OtherOperations()
	{
		model.SetupModel(shader_info);

	}

	void TestModel::SendDataToNextNode()
	{

	}

	void TestModel::ImguiRender()
	{

	}

}