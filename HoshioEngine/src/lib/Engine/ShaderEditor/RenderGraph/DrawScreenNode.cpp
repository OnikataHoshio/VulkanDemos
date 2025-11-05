#include "Engine/ShaderEditor/RenderGraph/DrawScreenNode.h"

namespace HoshioEngine {
	void DrawScreenNode::UpdateDescriptorSets()
	{
		switch (texturePicker.textureMode)
		{
		case TexturePicker::TextureMode::NO_MODE_SELECTED:
			throw std::runtime_error("[ DrawScreenNode ]ERROR\n DrawScreenNode doesn't have a texture to sample!");
			break;
		case TexturePicker::TextureMode::COLOR_ATTACHMENT_MODE:
			descriptorSet.Write(texturePicker.colorAttachment->DescriptorImageInfo(sampler), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);
			break;
		case TexturePicker::TextureMode::TEXTURE_MODE:
			descriptorSet.Write(texturePicker.texture->DescriptorImageInfo(sampler), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);
			break;
		default:
			break;
		}
	}

	void DrawScreenNode::RecordCommandBuffer()
	{
		const CommandBuffer& commandBuffer = VulkanPlus::Plus().CommandBuffer_Graphics();
		VulkanPlus::Plus().SwapchainRenderPass().Begin(commandBuffer, VulkanPlus::Plus().CurrentSwapchainFramebuffer(), { {},VulkanBase::Base().SwapchainCi().imageExtent }, { {1.0f} });
		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, VulkanPlus::Plus().DefaultVertexBuffer().Address(), &offset);
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, 8, &pushConstant);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[0]);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
			descriptorSet.Address(), 0, nullptr);
		vkCmdDraw(commandBuffer, 4, 1, 0, 0);
		VulkanPlus::Plus().SwapchainRenderPass().End(commandBuffer);
	}

	DrawScreenNode::DrawScreenNode(VkSampler sampler):sampler(sampler){}

	DrawScreenNode::DrawScreenNode(VkSampler sampler, ColorAttachment* colorAttachment):sampler(sampler)
	{
		SetSampledImage(colorAttachment);
	}

	DrawScreenNode::DrawScreenNode(VkSampler sampler, Texture2D* texture):sampler(sampler)
	{
		SetSampledImage(texture);
	}

	DrawScreenNode::DrawScreenNode(VkSampler sampler, Texture2D& texture):sampler(sampler)
	{
		SetSampledImage(&texture);
	}

	uint32_t DrawScreenNode::TextureMaxLevel() const
	{
		switch (texturePicker.textureMode)
		{
		case TexturePicker::TextureMode::NO_MODE_SELECTED:
			throw std::runtime_error("[ DrawScreenNode ]ERROR\n DrawScreenNode doesn't have a texture to sample!");
			break;
		case TexturePicker::TextureMode::COLOR_ATTACHMENT_MODE:
			return texturePicker.colorAttachment->MipLevelCount() - 1;
			break;
		case TexturePicker::TextureMode::TEXTURE_MODE:
			return texturePicker.texture->MipLevelCount() - 1;
			break;
		default:
			return 0;
			break;
		}
	}

	DrawScreenNode::PushConstant& DrawScreenNode::NodeParameter() 
	{
		return pushConstant;
	}

	void DrawScreenNode::SetSampledImage(ColorAttachment* colorAttachment)
	{
		if (colorAttachment == nullptr)
			return;
		texturePicker.colorAttachment = colorAttachment;
		texturePicker.textureMode = TexturePicker::TextureMode::COLOR_ATTACHMENT_MODE;
	}

	void DrawScreenNode::SetSampledImage(Texture2D* texture)
	{
		if (texture == nullptr)
			return;
		texturePicker.texture = texture;
		texturePicker.textureMode = TexturePicker::TextureMode::TEXTURE_MODE;
	}

	void DrawScreenNode::SendDataToNextNode()
	{
	}

	void DrawScreenNode::ImguiRender()
	{
	}

	void DrawScreenNode::InitResource()
	{
	}

	void DrawScreenNode::CreateSampler()
	{
	}

	void DrawScreenNode::CreateRenderPass()
	{
	}

	void DrawScreenNode::CreateDescriptorSetLayout()
	{
		//DescriptorSetLayout
		{
			VkDescriptorSetLayoutBinding binding = {
				.binding = 0,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.descriptorCount = 1,
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
			};
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
				.bindingCount = 1,
				.pBindings = &binding,
			};
			descriptorSetLayout.Create(descriptorSetLayoutCreateInfo);
		}

		{
			VulkanPlus::Plus().DescriptorPool().AllocateDescriptorSets(descriptorSet, descriptorSetLayout);
		}
	}

	void DrawScreenNode::CreatePipelineLayout()
	{
		//PipelineLayout
		{
			VkPushConstantRange pushConstantRange = {
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
				.offset = 0,
				.size = 8
			};

			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
				.setLayoutCount = uint32_t(1),
				.pSetLayouts = descriptorSetLayout.Address(),
				.pushConstantRangeCount = 1,
				.pPushConstantRanges = &pushConstantRange
			};

			pipelineLayout.Create(pipelineLayoutCreateInfo);
		}
	}

	void DrawScreenNode::CreatePipeline()
	{
		//Pipeline
		{
			auto Create = [&] {
				static ShaderModule vertModule("res/shaders/SPIR-V/default.vert.spv");
				static ShaderModule fragModule("res/shaders/SPIR-V/default.frag.spv");

				PipelineConfigurator configurator;
				configurator.PipelineLayout(pipelineLayout);
				configurator.RenderPass(VulkanPlus::Plus().SwapchainRenderPass());
				configurator.AddVertexInputBindings(0, sizeof(DefaultVertex), VK_VERTEX_INPUT_RATE_VERTEX);
				configurator.AddVertexInputAttribute(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(DefaultVertex, position));
				configurator.AddVertexInputAttribute(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(DefaultVertex, texCoord));
				configurator.PrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
				configurator.AddViewport(0.0f, 0.0f, static_cast<float>(VulkanBase::Base().SwapchainCi().imageExtent.width), static_cast<float>(VulkanBase::Base().SwapchainCi().imageExtent.height), 0.0f, 1.0f);
				configurator.AddScissor(VkOffset2D{}, VulkanBase::Base().SwapchainCi().imageExtent);
				configurator.RasterizationSamples(VK_SAMPLE_COUNT_1_BIT);
				configurator.AddAttachmentState(0b1111);
				configurator.AddShaderStage(vertModule.ShaderStageCi(VK_SHADER_STAGE_VERTEX_BIT));
				configurator.AddShaderStage(fragModule.ShaderStageCi(VK_SHADER_STAGE_FRAGMENT_BIT));
				configurator.UpdatePipelineCreateInfo();
				//configurator.PrintPipelineCreateInfo("DrawScreenNodePipeline CreateInfo:");
				pipelines.emplace_back(configurator);
				};

			auto Destroy = [&] {
				pipelines.clear();
				};

			VulkanBase::Base().AddCallback_CreateSwapchain(Create);
			VulkanBase::Base().AddCallback_DestroySwapchain(Destroy);

			Create();
		}
	}

	void DrawScreenNode::CreateFramebuffers()
	{
	}

	void DrawScreenNode::OtherOperations()
	{
	}

	void DrawScreenNode::CreateBuffer()
	{
	}

}