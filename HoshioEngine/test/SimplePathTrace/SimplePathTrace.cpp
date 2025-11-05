#include "SimplePathTrace.h"

namespace HoshioEngine {
	
	void SimplePathTrace::UpdateDescriptorSets()
	{
		static UniformBuffer uniformBuffer(sizeof u_Attribute);
		uniformBuffer.TransferData(&u_Attribute, sizeof u_Attribute);
		VkDescriptorBufferInfo bufferInfo = {
			.buffer = uniformBuffer,
			.offset = 0,
			.range = sizeof u_Attribute
		};
		uint32_t imageIndex = VulkanBase::Base().CurrentImageIndex();
		descriptorSet.Write(texture->DescriptorImageInfo(sampler), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);
		descriptorSet.Write(bufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
	}
	void SimplePathTrace::RecordCommandBuffer()
	{
		const CommandBuffer& commandBuffer = VulkanPlus::Plus().CommandBuffer_Graphics();
		VulkanPlus::Plus().SwapchainRenderPass().Begin(commandBuffer, VulkanPlus::Plus().CurrentSwapchainFramebuffer(), { {},VulkanBase::Base().SwapchainCi().imageExtent }, { {1.0f} });
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, VulkanPlus::Plus().DefaultVertexBuffer().Address(), &offset);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
			descriptorSet.Address(), 0, nullptr);
		vkCmdDraw(commandBuffer, 4, 1, 0, 0);
		VulkanPlus::Plus().SwapchainRenderPass().End(commandBuffer);
	}
	void SimplePathTrace::ImguiRender()
	{
		static bool _isOrthographic = false;
		ImGui::Checkbox(reinterpret_cast<const char*>(u8"正交投影"), &_isOrthographic);
		u_Attribute.isOrthographic = _isOrthographic ? 1 : 0;

		ImGui::SliderFloat(reinterpret_cast<const char*>(u8"kd"), &u_Attribute.kd, 0.0f, 1.0f);
		ImGui::SliderFloat(reinterpret_cast<const char*>(u8"面光源宽度(半)"), &u_Attribute.light_w, 0.1f, 1.0f);
		ImGui::SliderFloat(reinterpret_cast<const char*>(u8"面光源高度(半)"), &u_Attribute.light_h, 0.1f, 1.0f);
		ImGui::SliderFloat(reinterpret_cast<const char*>(u8"面光源位置"), &u_Attribute.light_pos, -25.0f, -0.1f);
		ImGui::SliderFloat(reinterpret_cast<const char*>(u8"面光源x偏移"), &u_Attribute.light_offset_x, -10.0f, 10.0f);
		ImGui::SliderFloat(reinterpret_cast<const char*>(u8"面光源y偏移"), &u_Attribute.light_offset_y, -10.0f, 10.0f);
		ImGui::SliderFloat(reinterpret_cast<const char*>(u8"面光源强度"), &u_Attribute.light_power, 0.1f, 1024.0f);

		ImGui::SliderFloat(reinterpret_cast<const char*>(u8"摄像机位置"), &u_Attribute.view_pos, -25.0f, -0.1f);
		ImGui::SliderFloat(reinterpret_cast<const char*>(u8"投影面位置"), &u_Attribute.panel_pos, 0.1f, 10.0f);

		ImGui::SliderInt(reinterpret_cast<const char*>(u8"采样次数"), reinterpret_cast<int*>(&u_Attribute.sample_count), 1, 1024);
	}

	SimplePathTrace::SimplePathTrace(VkSampler sampler, Texture2D* texture) :sampler(sampler), texture(texture)
	{
	}
	void SimplePathTrace::InitResource()
	{
	}
	void SimplePathTrace::SendDataToNextNode()
	{
	}
	void SimplePathTrace::CreateSampler()
	{
	}
	void SimplePathTrace::CreateRenderPass()
	{
	}
	void SimplePathTrace::CreateDescriptorSetLayout()
	{
		//DescriptorSetLayout
		{
			VkDescriptorSetLayoutBinding bindings[2] = {
				{
					.binding = 0,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 1,
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
				},
				{
					.binding = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					.descriptorCount = 1,
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
				}
			};
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
				.bindingCount = 2,
				.pBindings = bindings,
			};
			descriptorSetLayout.Create(descriptorSetLayoutCreateInfo);
		}

		{
			VulkanPlus::Plus().DescriptorPool().AllocateDescriptorSets(descriptorSet, descriptorSetLayout);
		}
	}
	void SimplePathTrace::CreatePipelineLayout()
	{
		//PipelineLayout
		{
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
				.setLayoutCount = 1,
				.pSetLayouts = descriptorSetLayout.Address(),
			};
			pipelineLayout.Create(pipelineLayoutCreateInfo);
		}
	}
	void SimplePathTrace::CreatePipeline()
	{
		{
			ShaderModule vertModule("test/SimplePathTrace/Resource/shaders/SPIR-V/default.vert.spv");
			ShaderModule fragModule("test/SimplePathTrace/Resource/shaders/SPIR-V/SimplePathTrace.frag.spv");

			PipelineConfigurator configurator;
			configurator.PipelineLayout(pipelineLayout)
				.RenderPass(VulkanPlus::Plus().SwapchainRenderPass())
				.AddVertexInputBindings(0, sizeof(DefaultVertex), VK_VERTEX_INPUT_RATE_VERTEX)
				.AddVertexInputAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(DefaultVertex, position))
				.AddVertexInputAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(DefaultVertex, texCoord))
				.PrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)
				.AddViewport(0.0f, 0.0f, static_cast<float>(VulkanBase::Base().SwapchainCi().imageExtent.width), static_cast<float>(VulkanBase::Base().SwapchainCi().imageExtent.height), 0.0f, 1.0f)
				.AddScissor(VkOffset2D{}, VulkanBase::Base().SwapchainCi().imageExtent)
				.RasterizationSamples(VK_SAMPLE_COUNT_1_BIT)
				.AddAttachmentState(0b1111)
				.AddShaderStage(vertModule.ShaderStageCi(VK_SHADER_STAGE_VERTEX_BIT))
				.AddShaderStage(fragModule.ShaderStageCi(VK_SHADER_STAGE_FRAGMENT_BIT))
				.UpdatePipelineCreateInfo();
			//configurator.PrintPipelineCreateInfo("DrawScreenNodePipeline CreateInfo:");
			pipeline.Create(configurator);
		}
	}
	void SimplePathTrace::CreateFramebuffers()
	{
	}
	void SimplePathTrace::OtherOperations()
	{
	}
	void SimplePathTrace::CreateBuffer()
	{
	}
}

