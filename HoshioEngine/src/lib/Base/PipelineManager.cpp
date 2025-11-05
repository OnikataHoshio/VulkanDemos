#include "Base/PipelineManager.h"

namespace HoshioEngine {

#pragma region ShaderModule

	ShaderModule::ShaderModule(VkShaderModuleCreateInfo& createInfo)
	{
		Create(createInfo);
	}
	ShaderModule::ShaderModule(const char* filePath)
	{
		Create(filePath);
	}
	ShaderModule::ShaderModule(size_t codeSize, const uint32_t* pCode)
	{
		Create(codeSize, pCode);
	}
	ShaderModule::ShaderModule(ShaderModule&& other) noexcept
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}
	ShaderModule::~ShaderModule()
	{
		if (handle) {
			vkDestroyShaderModule(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}
	ShaderModule::operator VkShaderModule() const
	{
		return handle;
	}
	const VkShaderModule* ShaderModule::Address() const
	{
		return &handle;
	}
	VkPipelineShaderStageCreateInfo ShaderModule::ShaderStageCi(VkShaderStageFlagBits stage, const char* entry) const
	{
		VkPipelineShaderStageCreateInfo shaderStageCi = {};
		shaderStageCi.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageCi.pName = nullptr;
		shaderStageCi.flags = 0;
		shaderStageCi.stage = stage;
		shaderStageCi.pName = entry;
		shaderStageCi.module = handle;
		shaderStageCi.pSpecializationInfo = nullptr;
		return shaderStageCi;
	}
	void ShaderModule::Create(VkShaderModuleCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		if (vkCreateShaderModule(VulkanBase::Base().Device(), &createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a shader module create info");
	}
	void ShaderModule::Create(const char* filePath)
	{
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);
		if (!file) {
			std::cout << std::format("[ Shader ] ERROR \nFailed to open the file : {}\n", filePath);
			throw std::runtime_error(std::format("Failed to open the file : {}", filePath));
		}
		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<uint32_t> binaries(fileSize / 4);
		file.seekg(0);
		file.read(reinterpret_cast<char*>(binaries.data()), fileSize);
		file.close();
		return Create(fileSize, binaries.data());
	}
	void ShaderModule::Create(size_t codeSize, const uint32_t* pCode)
	{
		VkShaderModuleCreateInfo createInfo = {
			.codeSize = codeSize,
			.pCode = pCode
		};
		Create(createInfo);
	}

#pragma endregion

#pragma region PipelineLayout

	PipelineLayout::PipelineLayout(VkPipelineLayoutCreateInfo& createInfo)
	{
		Create(createInfo);
	}

	PipelineLayout::PipelineLayout(PipelineLayout&& other)
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}

	PipelineLayout::~PipelineLayout()
	{
		if (handle) {
			vkDestroyPipelineLayout(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}

	PipelineLayout::operator VkPipelineLayout() const
	{
		return handle;
	}

	const VkPipelineLayout* PipelineLayout::Address() const
	{
		return &handle;
	}

	void PipelineLayout::Create(VkPipelineLayoutCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		if (vkCreatePipelineLayout(VulkanBase::Base().Device(), &createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create pipeline layout");
	}

#pragma endregion

#pragma region Pipeline

	Pipeline::Pipeline(VkGraphicsPipelineCreateInfo& createInfo)
	{
		Create(createInfo);
	}

	Pipeline::Pipeline(VkComputePipelineCreateInfo& createInfo)
	{
		Create(createInfo);
	}

	Pipeline::Pipeline(Pipeline&& other)
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}

	Pipeline::~Pipeline()
	{
		if (handle) {
			vkDestroyPipeline(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}

	Pipeline::operator VkPipeline() const
	{
		return handle;
	}

	const VkPipeline* Pipeline::Address() const
	{
		return &handle;
	}

	void Pipeline::Create(VkGraphicsPipelineCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		if (vkCreateGraphicsPipelines(VulkanBase::Base().Device(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a graphics pipelines");
	}

	void Pipeline::Create(VkComputePipelineCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		if (vkCreateComputePipelines(VulkanBase::Base().Device(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a compute pipelines");
	}

#pragma endregion

#pragma region PipelineConfigurator

	PipelineConfigurator::PipelineConfigurator()
	{
		BindFixFunctionCi();
		createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		createInfo.basePipelineHandle = VK_NULL_HANDLE;
		createInfo.basePipelineIndex = -1;

		vertexInputStateCi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		inputAssemblyStateCi.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		tessellationStateCi.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		viewportStateCi.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		rasterizationStateCi.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		multisampleStateCi.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		depthStencilStateCi.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		colorBlendStateCi.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		dynamicStateCi.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	}

	PipelineConfigurator::PipelineConfigurator(const PipelineConfigurator& other) noexcept
	{
		createInfo = other.createInfo;
		BindFixFunctionCi();

		vertexInputStateCi = other.vertexInputStateCi;
		inputAssemblyStateCi = other.inputAssemblyStateCi;
		tessellationStateCi = other.tessellationStateCi;
		viewportStateCi = other.viewportStateCi;
		rasterizationStateCi = other.rasterizationStateCi;
		multisampleStateCi = other.multisampleStateCi;
		depthStencilStateCi = other.depthStencilStateCi;
		colorBlendStateCi = other.colorBlendStateCi;
		dynamicStateCi = other.dynamicStateCi;

		shaderStages = other.shaderStages;
		vertexInputBindings = other.vertexInputBindings;
		vertexInputAttributes = other.vertexInputAttributes;
		viewports = other.viewports;
		scissors = other.scissors;
		colorBlendAttachmentStates = other.colorBlendAttachmentStates;
		dynamicStates = other.dynamicStates;
		
		UpdatePipelineCreateInfo();
	}

	PipelineConfigurator::operator VkGraphicsPipelineCreateInfo() const
	{
		return createInfo;
	}

	PipelineConfigurator::operator VkGraphicsPipelineCreateInfo& ()
	{
		return createInfo;
	}



	PipelineConfigurator& PipelineConfigurator::PipelineLayout(VkPipelineLayout pipelineLayout)
	{
		createInfo.layout = pipelineLayout;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::RenderPass(VkRenderPass renderPass)
	{
		createInfo.renderPass = renderPass;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::Subpass(uint32_t subpass_index)
	{
		createInfo.subpass = subpass_index;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::BasePipelineHandle(VkPipeline basePipeline)
	{
		createInfo.basePipelineHandle = basePipeline;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::BasePipelineIndex(int32_t basePipelineIndex)
	{
		createInfo.basePipelineIndex = basePipelineIndex;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::AddShaderStage(VkPipelineShaderStageCreateInfo shaderStage)
	{
		shaderStages.push_back(shaderStage);
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::AddVertexInputBindings(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate)
	{
		vertexInputBindings.push_back({ binding,stride,inputRate });
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::AddVertexInputAttribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset)
	{
		vertexInputAttributes.push_back({ location, binding, format, offset });
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::PrimitiveTopology(VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable)
	{
		inputAssemblyStateCi.topology = topology;
		inputAssemblyStateCi.primitiveRestartEnable = primitiveRestartEnable;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::TessPatchControlPoints(uint32_t controlPointCount)
	{
		tessellationStateCi.patchControlPoints = controlPointCount;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::AddViewport(float x, float y, float width, float height, float minDepth, float maxDepth)
	{
		viewports.push_back({ x, y, width, height, minDepth, maxDepth });
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::AddScissor(VkOffset2D offset, VkExtent2D extent)
	{
		scissors.push_back({ offset, extent });
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::EnableDepthClamp(VkBool32 depthClampEnable)
	{
		rasterizationStateCi.depthClampEnable = depthClampEnable;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::EnableRasterizerDiscard(VkBool32 discardRasterizer)
	{
		rasterizationStateCi.rasterizerDiscardEnable = discardRasterizer;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::PolygonMode(VkPolygonMode polygonMode)
	{
		rasterizationStateCi.polygonMode = polygonMode;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::CullMode(VkCullModeFlags cullMode)
	{
		rasterizationStateCi.cullMode = cullMode;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::FrontFace(VkFrontFace frontFace)
	{
		rasterizationStateCi.frontFace = frontFace;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::LineWidth(float lineWidth)
	{
		rasterizationStateCi.lineWidth = lineWidth;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::EnableDepthBias(VkBool32 depthBiasEnable, float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
	{
		rasterizationStateCi.depthBiasEnable = depthBiasEnable;
		rasterizationStateCi.depthBiasConstantFactor = depthBiasConstantFactor;
		rasterizationStateCi.depthBiasClamp = depthBiasClamp;
		rasterizationStateCi.depthBiasSlopeFactor = depthBiasSlopeFactor;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::RasterizationSamples(VkSampleCountFlagBits rasterizationSamples)
	{
		multisampleStateCi.rasterizationSamples = rasterizationSamples;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::EnableSampleShading(VkBool32 sampleShadingEnable)
	{
		multisampleStateCi.sampleShadingEnable = sampleShadingEnable;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::EnableDepthTest(VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp)
	{
		depthStencilStateCi.depthTestEnable = depthTestEnable;
		depthStencilStateCi.depthWriteEnable = depthWriteEnable;
		depthStencilStateCi.depthCompareOp = depthCompareOp;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::EnableDepthBoundsTest(VkBool32 depthBoundsTestEnable, float minDepthBounds, float maxDepthBounds)
	{
		depthStencilStateCi.depthBoundsTestEnable = depthBoundsTestEnable;
		depthStencilStateCi.minDepthBounds = minDepthBounds;
		depthStencilStateCi.maxDepthBounds = maxDepthBounds;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::EnableStencilTest(VkBool32 stencilTestEnable, VkStencilOpState& front, VkStencilOpState& back)
	{
		depthStencilStateCi.stencilTestEnable = stencilTestEnable;
		depthStencilStateCi.front = front;
		depthStencilStateCi.back = back;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::EnableLogicOp(VkBool32 logicOpEnable, VkLogicOp logicOp)
	{
		colorBlendStateCi.logicOpEnable = logicOpEnable;
		colorBlendStateCi.logicOp = logicOp;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::AddAttachmentState(VkColorComponentFlags colorWriteMask)
	{
		colorBlendAttachmentStates.push_back({ .colorWriteMask = colorWriteMask });
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::EnableBlend(VkBool32 blendEnable, uint32_t attachmentIndex)
	{
		if (attachmentIndex >= colorBlendAttachmentStates.size()) {
			std::cout << std::format("[ Pipeline ] WARNING\n attachmentIndex out of the range of color Blend Attachment States!\n");
			return *this;
		}
		colorBlendAttachmentStates[attachmentIndex].blendEnable = blendEnable;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::EnableBlend(VkBool32 blendEnable)
	{
		for (uint32_t i = 0; i < colorBlendAttachmentStates.size(); i++)
			EnableBlend(blendEnable, i);
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::ColorBlend(uint32_t attachmentIndex, VkBlendFactor srcColorBlendFactor, VkBlendFactor dstColorBlendFactor, VkBlendOp colorBlendOp)
	{
		if (attachmentIndex >= colorBlendAttachmentStates.size()) {
			std::cout << std::format("[ Pipeline ] WARNING\n attachmentIndex out of the range of color Blend Attachment States!\n");
			return *this;
		}
		colorBlendAttachmentStates[attachmentIndex].srcColorBlendFactor = srcColorBlendFactor;
		colorBlendAttachmentStates[attachmentIndex].dstColorBlendFactor = dstColorBlendFactor;
		colorBlendAttachmentStates[attachmentIndex].colorBlendOp = colorBlendOp;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::ColorBlend(VkBlendFactor srcColorBlendFactor, VkBlendFactor dstColorBlendFactor, VkBlendOp colorBlendOp)
	{
		for (uint32_t i = 0; i < colorBlendAttachmentStates.size(); i++)
			ColorBlend(i, srcColorBlendFactor, dstColorBlendFactor, colorBlendOp);
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::AlphaBlend(uint32_t attachmentIndex, VkBlendFactor srcAlphaBlendFactor, VkBlendFactor dstAlphaBlendFactor, VkBlendOp alphaBlendOp)
	{
		if (attachmentIndex >= colorBlendAttachmentStates.size()) {
			std::cout << std::format("[ Pipeline ] WARNING\n attachmentIndex out of the range of color Blend Attachment States!\n");
			return *this;
		}
		colorBlendAttachmentStates[attachmentIndex].srcAlphaBlendFactor = srcAlphaBlendFactor;
		colorBlendAttachmentStates[attachmentIndex].dstAlphaBlendFactor = dstAlphaBlendFactor;
		colorBlendAttachmentStates[attachmentIndex].alphaBlendOp = alphaBlendOp;
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::AlphaBlend(VkBlendFactor srcAlphaBlendFactor, VkBlendFactor dstAlphaBlendFactor, VkBlendOp alphaBlendOp)
	{
		for (uint32_t i = 0; i < colorBlendAttachmentStates.size(); i++)
			AlphaBlend(i, srcAlphaBlendFactor, dstAlphaBlendFactor, alphaBlendOp);
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::AddDynamicState(VkDynamicState dynamicState)
	{
		dynamicStates.push_back(dynamicState);
		return *this;
	}

	PipelineConfigurator& PipelineConfigurator::UpdatePipelineCreateInfo()
	{
		createInfo.stageCount = shaderStages.size();
		vertexInputStateCi.vertexBindingDescriptionCount = vertexInputBindings.size();
		vertexInputStateCi.vertexAttributeDescriptionCount = vertexInputAttributes.size();
		viewportStateCi.viewportCount = viewports.size() ? viewports.size() : 1;
		viewportStateCi.scissorCount = scissors.size() ? scissors.size() : 1;
		colorBlendStateCi.attachmentCount = colorBlendAttachmentStates.size();
		dynamicStateCi.dynamicStateCount = dynamicStates.size();
		UpdateCiInnerPointer();
		return *this;
	}

	void PipelineConfigurator::BindFixFunctionCi()
	{
		createInfo.pVertexInputState = &vertexInputStateCi;
		createInfo.pInputAssemblyState = &inputAssemblyStateCi;
		createInfo.pTessellationState = &tessellationStateCi;
		createInfo.pViewportState = &viewportStateCi;
		createInfo.pRasterizationState = &rasterizationStateCi;
		createInfo.pMultisampleState = &multisampleStateCi;
		createInfo.pDepthStencilState = &depthStencilStateCi;
		createInfo.pColorBlendState = &colorBlendStateCi;
		createInfo.pDynamicState = &dynamicStateCi;
	}

	void PipelineConfigurator::UpdateCiInnerPointer()
	{
		createInfo.pStages = shaderStages.data();
		vertexInputStateCi.pVertexBindingDescriptions = vertexInputBindings.data();
		vertexInputStateCi.pVertexAttributeDescriptions = vertexInputAttributes.data();
		viewportStateCi.pViewports = viewports.data();
		viewportStateCi.pScissors = scissors.data();
		colorBlendStateCi.pAttachments = colorBlendAttachmentStates.data();
		dynamicStateCi.pDynamicStates = dynamicStates.data();
	}

	void PipelineConfigurator::PrintPipelineCreateInfo(const char* tip) const
	{
		std::cout << std::format("\n{}\n", tip);
		std::cout << std::format("    sType : {}\n", magic_enum::enum_name(createInfo.sType));
		std::cout << std::format("    pNext : {}\n", createInfo.pNext);
		std::cout << std::format("    flags : 0x{:08X}\n", createInfo.flags);
		std::cout << std::format("    stageCount : {}\n", createInfo.stageCount);

		print_pStages();
		print_pVertexInputState();
		print_pInputAssemblyState();
		print_pTessellationState();
		print_pViewportState();
		print_pDepthStencilState();
		print_pMultisampleState();
		print_pDepthStencilState();
		print_pColorBlendState();
		print_pDynamicState();

		std::cout << std::format("    pipelineLayout : {}\n", static_cast<void*>(createInfo.layout));
		std::cout << std::format("    renderPass : {}\n", static_cast<void*>(createInfo.renderPass));
		std::cout << std::format("    subpass : {}\n", createInfo.subpass);
		std::cout << std::format("    basePipelineHandle : {}\n", static_cast<void*>(createInfo.basePipelineHandle));
		std::cout << std::format("    basePipelineIndex : {}\n", createInfo.basePipelineIndex);

		std::cout << std::format("{}\n", "End");
	}

	void PipelineConfigurator::print_pStages() const
	{
		const char* prefix = "        ";
		std::cout << std::format("    pStage[{}] :\n", createInfo.stageCount);
		for (uint32_t i = 0; i < createInfo.stageCount; i++) {
			std::cout << "      {\n";
			std::cout << std::format("{}sType : {}\n", prefix, magic_enum::enum_name(createInfo.pStages[i].sType));
			std::cout << std::format("{}pNext : {}\n", prefix, createInfo.pStages[i].pNext);
			std::cout << std::format("{}flags : 0x{:08X}\n", prefix, createInfo.pStages[i].flags);
			std::cout << std::format("{}stage : {}\n", prefix, magic_enum::enum_name(createInfo.pStages[i].stage));
			std::cout << std::format("{}module : {}\n", prefix, static_cast<void*>(createInfo.pStages[i].module));
			std::cout << std::format("{}name : \"{}\"\n", prefix, createInfo.pStages[i].pName);
			std::cout << std::format("{}pSpecializationInfo : nullptr\n", prefix);
			if (i == createInfo.stageCount - 1)
				std::cout << "      }\n";
			else
				std::cout << "      },\n";
		}
	}

	void PipelineConfigurator::print_pVertexInputState() const
	{
		const char* prefix = "        ";
		std::cout << std::format("    pVertexInputState :\n");
		std::cout << "      {\n";

		std::cout << std::format("{}sType : {}\n", prefix, magic_enum::enum_name(createInfo.pVertexInputState->sType));
		std::cout << std::format("{}pNext : {}\n", prefix, createInfo.pVertexInputState->pNext);
		std::cout << std::format("{}flags : 0x{:08X}\n", prefix, createInfo.pVertexInputState->flags);
		std::cout << std::format("{}vertexBindingDescriptionCount : {}\n", prefix, createInfo.pVertexInputState->vertexBindingDescriptionCount);
		print_pVertexBindingDescriptions();
		std::cout << std::format("{}vertexAttributeDescriptionCount : {}\n", prefix, createInfo.pVertexInputState->vertexAttributeDescriptionCount);
		print_pVertexAttributeDescriptions();

		std::cout << "      }\n";
	}

	void PipelineConfigurator::print_pVertexBindingDescriptions() const
	{
		const char* prefix = "            ";
		std::cout << std::format("        pVertexBindingDescriptions[{}] :\n", createInfo.pVertexInputState->vertexBindingDescriptionCount);
		for (uint32_t i = 0; i < createInfo.pVertexInputState->vertexBindingDescriptionCount; i++) {
			std::cout << "          {\n";

			std::cout << std::format("{}binding : {}\n", prefix, createInfo.pVertexInputState->pVertexBindingDescriptions[i].binding);
			std::cout << std::format("{}stride : {}\n", prefix, createInfo.pVertexInputState->pVertexBindingDescriptions[i].stride);
			std::cout << std::format("{}inputRate : {}\n", prefix, magic_enum::enum_name(createInfo.pVertexInputState->pVertexBindingDescriptions[i].inputRate));

			if (i == createInfo.pVertexInputState->vertexBindingDescriptionCount - 1)
				std::cout << "          }\n";
			else
				std::cout << "          },\n";
		}
	}

	void PipelineConfigurator::print_pVertexAttributeDescriptions() const
	{
		const char* prefix = "            ";
		std::cout << std::format("        pVertexAttributeDescriptions[{}] :\n", createInfo.pVertexInputState->vertexAttributeDescriptionCount);
		for (uint32_t i = 0; i < createInfo.pVertexInputState->vertexAttributeDescriptionCount; i++) {
			std::cout << "          {\n";

			std::cout << std::format("{}location : {}\n", prefix, createInfo.pVertexInputState->pVertexAttributeDescriptions[i].location);
			std::cout << std::format("{}binding : {}\n", prefix, createInfo.pVertexInputState->pVertexAttributeDescriptions[i].binding);
			std::cout << std::format("{}format : {}\n", prefix, magic_enum::enum_name(createInfo.pVertexInputState->pVertexAttributeDescriptions[i].format));
			std::cout << std::format("{}offset : {}\n", prefix, createInfo.pVertexInputState->pVertexAttributeDescriptions[i].offset);

			if (i == createInfo.pVertexInputState->vertexAttributeDescriptionCount - 1)
				std::cout << "          }\n";
			else
				std::cout << "          },\n";
		}
	}

	void PipelineConfigurator::print_pInputAssemblyState() const
	{
		const char* prefix = "        ";
		std::cout << std::format("    pInputAssemblyState :\n");
		std::cout << "      {\n";

		std::cout << std::format("{}sType : {}\n", prefix, magic_enum::enum_name(createInfo.pInputAssemblyState->sType));
		std::cout << std::format("{}pNext : {}\n", prefix, createInfo.pInputAssemblyState->pNext);
		std::cout << std::format("{}flags : 0x{:08X}\n", prefix, createInfo.pInputAssemblyState->flags);
		std::cout << std::format("{}topology : {}\n", prefix, magic_enum::enum_name(createInfo.pInputAssemblyState->topology));
		std::cout << std::format("{}primitiveRestartEnable : {}\n", prefix, createInfo.pInputAssemblyState->primitiveRestartEnable ? "true" : "false");

		std::cout << "      }\n";
	}

	void PipelineConfigurator::print_pTessellationState() const
	{
		const char* prefix = "        ";
		std::cout << std::format("    pTessellationState :\n");
		std::cout << "      {\n";

		std::cout << std::format("{}sType : {}\n", prefix, magic_enum::enum_name(createInfo.pTessellationState->sType));
		std::cout << std::format("{}pNext : {}\n", prefix, createInfo.pTessellationState->pNext);
		std::cout << std::format("{}flags : 0x{:08X}\n", prefix, createInfo.pTessellationState->flags);
		std::cout << std::format("{}patchControlPoints : {}\n", prefix, createInfo.pTessellationState->patchControlPoints);

		std::cout << "      }\n";
	}

	void PipelineConfigurator::print_pViewportState() const
	{
		const char* prefix = "        ";
		std::cout << std::format("    pViewportState :\n");
		std::cout << "      {\n";

		std::cout << std::format("{}sType : {}\n", prefix, magic_enum::enum_name(createInfo.pViewportState->sType));
		std::cout << std::format("{}pNext : {}\n", prefix, createInfo.pViewportState->pNext);
		std::cout << std::format("{}flags : 0x{:08X}\n", prefix, createInfo.pViewportState->flags);
		std::cout << std::format("{}viewportCount : {}\n", prefix, createInfo.pViewportState->viewportCount);
		print_pViewports();
		std::cout << std::format("{}scissorCount : {}\n", prefix, createInfo.pViewportState->scissorCount);
		print_pScissors();

		std::cout << "      }\n";
	}

	void PipelineConfigurator::print_pViewports() const
	{
		const char* prefix = "            ";
		std::cout << std::format("        pViewports[{}] :\n", createInfo.pViewportState->viewportCount);
		for (uint32_t i = 0; i < createInfo.pViewportState->viewportCount; i++) {
			std::cout << "          {\n";

			std::cout << std::format("{}x : {}\n", prefix, createInfo.pViewportState->pViewports[i].x);
			std::cout << std::format("{}y : {}\n", prefix, createInfo.pViewportState->pViewports[i].y);
			std::cout << std::format("{}width : {}\n", prefix, createInfo.pViewportState->pViewports[i].width);
			std::cout << std::format("{}height : {}\n", prefix, createInfo.pViewportState->pViewports[i].height);
			std::cout << std::format("{}minDepth : {}\n", prefix, createInfo.pViewportState->pViewports[i].minDepth);
			std::cout << std::format("{}maxDepth : {}\n", prefix, createInfo.pViewportState->pViewports[i].maxDepth);

			if (i == createInfo.pViewportState->viewportCount - 1)
				std::cout << "          }\n";
			else
				std::cout << "          },\n";
		}
	}

	void PipelineConfigurator::print_pScissors() const
	{
		const char* prefix = "            ";
		std::cout << std::format("        pViewports[{}] :\n", createInfo.pViewportState->scissorCount);
		for (uint32_t i = 0; i < createInfo.pViewportState->scissorCount; i++) {
			std::cout << "          {\n";

			std::cout << std::format("{}offset : \n", prefix);
			std::cout << "            {\n";
			std::cout << std::format("{}  x : {}\n", prefix, createInfo.pViewportState->pScissors[i].offset.x);
			std::cout << std::format("{}  y : {}\n", prefix, createInfo.pViewportState->pScissors[i].offset.y);
			std::cout << "            }\n";

			std::cout << std::format("{}extent : \n", prefix);
			std::cout << "            {\n";
			std::cout << std::format("{}  width : {}\n", prefix, createInfo.pViewportState->pScissors[i].extent.width);
			std::cout << std::format("{}  height : {}\n", prefix, createInfo.pViewportState->pScissors[i].extent.height);
			std::cout << "            }\n";

			if (i == createInfo.pViewportState->scissorCount - 1)
				std::cout << "          }\n";
			else
				std::cout << "          },\n";
		}
	}

	void PipelineConfigurator::print_pRasterizationState() const
	{
		const char* prefix = "        ";
		std::cout << std::format("    pRasterizationState :\n");
		std::cout << "      {\n";

		std::cout << std::format("{}sType : {}\n", prefix, magic_enum::enum_name(createInfo.pRasterizationState->sType));
		std::cout << std::format("{}pNext : {}\n", prefix, createInfo.pRasterizationState->pNext);
		std::cout << std::format("{}flags : 0x{:08X}\n", prefix, createInfo.pRasterizationState->flags);
		std::cout << std::format("{}depthClampEnable : {}\n", prefix, createInfo.pRasterizationState->depthClampEnable ? "true" : "false");
		std::cout << std::format("{}rasterizerDiscardEnable : {}\n", prefix, createInfo.pRasterizationState->rasterizerDiscardEnable ? "true" : "false");
		std::cout << std::format("{}polygonMode : {}\n", prefix, magic_enum::enum_name(createInfo.pRasterizationState->polygonMode));
		std::cout << std::format("{}cullMode : 0x{:08X}\n", prefix, createInfo.pRasterizationState->cullMode);
		std::cout << std::format("{}frontFace : {}\n", prefix, magic_enum::enum_name(createInfo.pRasterizationState->frontFace));
		std::cout << std::format("{}depthBiasEnable : {}\n", prefix, createInfo.pRasterizationState->depthBiasEnable ? "true" : "false");
		std::cout << std::format("{}depthBiasConstantFactor : {}\n", prefix, createInfo.pRasterizationState->depthBiasConstantFactor);
		std::cout << std::format("{}depthBiasClamp : {}\n", prefix, createInfo.pRasterizationState->depthBiasClamp);
		std::cout << std::format("{}depthBiasSlopeFactor : {}\n", prefix, createInfo.pRasterizationState->depthBiasSlopeFactor);
		std::cout << std::format("{}lineWidth : {}\n", prefix, createInfo.pRasterizationState->lineWidth);

		std::cout << "      }\n";
	}

	void PipelineConfigurator::print_pMultisampleState() const
	{
		const char* prefix = "        ";
		std::cout << std::format("    pMultisampleState :\n");
		std::cout << "      {\n";

		std::cout << std::format("{}sType : {}\n", prefix, magic_enum::enum_name(createInfo.pMultisampleState->sType));
		std::cout << std::format("{}pNext : {}\n", prefix, createInfo.pMultisampleState->pNext);
		std::cout << std::format("{}flags : 0x{:08X}\n", prefix, createInfo.pMultisampleState->flags);
		std::cout << std::format("{}rasterizationSamples : {}\n", prefix, magic_enum::enum_name(createInfo.pMultisampleState->rasterizationSamples));
		std::cout << std::format("{}sampleShadingEnable : {}\n", prefix, createInfo.pMultisampleState->sampleShadingEnable ? "true" : "false");
		std::cout << std::format("{}minSampleShading : {}\n", prefix, createInfo.pMultisampleState->minSampleShading);
		std::cout << std::format("{}pSampleMask : 0x{:08X}\n", prefix, createInfo.pMultisampleState->pSampleMask ? *createInfo.pMultisampleState->pSampleMask : 0);
		std::cout << std::format("{}alphaToCoverageEnable : {}\n", prefix, createInfo.pMultisampleState->alphaToCoverageEnable ? "true" : "false");
		std::cout << std::format("{}alphaToOneEnable : {}\n", prefix, createInfo.pMultisampleState->alphaToOneEnable ? "true" : "false");
		std::cout << "      }\n";

	}

	void PipelineConfigurator::print_pDepthStencilState() const
	{
		const char* prefix = "        ";
		std::cout << std::format("    pDepthStencilState :\n");
		std::cout << "      {\n";

		std::cout << std::format("{}sType : {}\n", prefix, magic_enum::enum_name(createInfo.pDepthStencilState->sType));
		std::cout << std::format("{}pNext : {}\n", prefix, createInfo.pDepthStencilState->pNext);
		std::cout << std::format("{}flags : 0x{:08X}\n", prefix, createInfo.pDepthStencilState->flags);
		std::cout << std::format("{}depthTestEnable : {}\n", prefix, createInfo.pDepthStencilState->depthTestEnable ? "true" : "false");
		std::cout << std::format("{}depthWriteEnable : {}\n", prefix, createInfo.pDepthStencilState->depthWriteEnable ? "true" : "false");
		std::cout << std::format("{}depthCompareOp : {}\n", prefix, magic_enum::enum_name(createInfo.pDepthStencilState->depthCompareOp));
		std::cout << std::format("{}depthBoundsTestEnable : {}\n", prefix, createInfo.pDepthStencilState->depthBoundsTestEnable ? "true" : "false");
		std::cout << std::format("{}stencilTestEnable : {}\n", prefix, createInfo.pDepthStencilState->stencilTestEnable ? "true" : "false");
		std::cout << std::format("{}depthCompareOp : {}\n", prefix, magic_enum::enum_name(createInfo.pDepthStencilState->depthCompareOp));

		std::cout << std::format("{}front : \n", prefix);
		std::cout << "          {\n";
		std::cout << std::format("{}    failOp : {}\n", prefix, magic_enum::enum_name(createInfo.pDepthStencilState->front.failOp));
		std::cout << std::format("{}    passOp : {}\n", prefix, magic_enum::enum_name(createInfo.pDepthStencilState->front.passOp));
		std::cout << std::format("{}    depthFailOp : {}\n", prefix, magic_enum::enum_name(createInfo.pDepthStencilState->front.depthFailOp));
		std::cout << std::format("{}    compareOp : {}\n", prefix, magic_enum::enum_name(createInfo.pDepthStencilState->front.compareOp));
		std::cout << std::format("{}    compareMask : {}\n", prefix, createInfo.pDepthStencilState->front.compareMask);
		std::cout << std::format("{}    writeMask : {}\n", prefix, createInfo.pDepthStencilState->front.writeMask);
		std::cout << std::format("{}    reference : {}\n", prefix, createInfo.pDepthStencilState->front.reference);
		std::cout << "          }\n";

		std::cout << std::format("{}back : \n", prefix);
		std::cout << "          {\n";
		std::cout << std::format("{}    failOp : {}\n", prefix, magic_enum::enum_name(createInfo.pDepthStencilState->back.failOp));
		std::cout << std::format("{}    passOp : {}\n", prefix, magic_enum::enum_name(createInfo.pDepthStencilState->back.passOp));
		std::cout << std::format("{}    depthFailOp : {}\n", prefix, magic_enum::enum_name(createInfo.pDepthStencilState->back.depthFailOp));
		std::cout << std::format("{}    compareOp : {}\n", prefix, magic_enum::enum_name(createInfo.pDepthStencilState->back.compareOp));
		std::cout << std::format("{}    compareMask : {}\n", prefix, createInfo.pDepthStencilState->back.compareMask);
		std::cout << std::format("{}    writeMask : {}\n", prefix, createInfo.pDepthStencilState->back.writeMask);
		std::cout << std::format("{}    reference : {}\n", prefix, createInfo.pDepthStencilState->back.reference);
		std::cout << "          }\n";

		std::cout << std::format("{}minDepthBounds : {}\n", prefix, createInfo.pDepthStencilState->minDepthBounds);
		std::cout << std::format("{}maxDepthBounds : {}\n", prefix, createInfo.pDepthStencilState->maxDepthBounds);

		std::cout << "      }\n";
	}

	void PipelineConfigurator::print_pColorBlendState() const
	{
		const char* prefix = "        ";
		std::cout << std::format("    pColorBlendState :\n");
		std::cout << "      {\n";

		std::cout << std::format("{}sType : {}\n", prefix, magic_enum::enum_name(createInfo.pColorBlendState->sType));
		std::cout << std::format("{}pNext : {}\n", prefix, createInfo.pColorBlendState->pNext);
		std::cout << std::format("{}flags : 0x{:08X}\n", prefix, createInfo.pColorBlendState->flags);
		std::cout << std::format("{}logicOpEnable : {}\n", prefix, createInfo.pColorBlendState->logicOpEnable ? "true" : "false");
		std::cout << std::format("{}logicOp : {}\n", prefix, magic_enum::enum_name(createInfo.pColorBlendState->logicOp));
		std::cout << std::format("{}attachmentCount : {}\n", prefix, createInfo.pColorBlendState->attachmentCount);

		print_pAttachments();

		std::cout << std::format("{}blendConstants[4] : {{ {}, {}, {}, {} }}\n", prefix, 
			createInfo.pColorBlendState->blendConstants[0], createInfo.pColorBlendState->blendConstants[1],
			createInfo.pColorBlendState->blendConstants[2], createInfo.pColorBlendState->blendConstants[3]);
		std::cout << "      }\n";
	}

	void PipelineConfigurator::print_pAttachments() const
	{
		const char* prefix = "            ";
		std::cout << std::format("        pAttachments[{}] :\n", createInfo.pColorBlendState->attachmentCount);
		for (uint32_t i = 0; i < createInfo.pColorBlendState->attachmentCount; i++) {
			std::cout << "          {\n";

			std::cout << std::format("{}blendEnable : {}\n", prefix, createInfo.pColorBlendState->pAttachments[i].blendEnable ? "true" : "false");
			std::cout << std::format("{}srcColorBlendFactor : {}\n", prefix, magic_enum::enum_name(createInfo.pColorBlendState->pAttachments[i].srcColorBlendFactor));
			std::cout << std::format("{}dstColorBlendFactor : {}\n", prefix, magic_enum::enum_name(createInfo.pColorBlendState->pAttachments[i].dstColorBlendFactor));
			std::cout << std::format("{}colorBlendOp : {}\n", prefix, magic_enum::enum_name(createInfo.pColorBlendState->pAttachments[i].colorBlendOp));

			std::cout << std::format("{}srcAlphaBlendFactor : {}\n", prefix, magic_enum::enum_name(createInfo.pColorBlendState->pAttachments[i].srcAlphaBlendFactor));
			std::cout << std::format("{}dstAlphaBlendFactor : {}\n", prefix, magic_enum::enum_name(createInfo.pColorBlendState->pAttachments[i].dstAlphaBlendFactor));
			std::cout << std::format("{}alphaBlendOp : {}\n", prefix, magic_enum::enum_name(createInfo.pColorBlendState->pAttachments[i].alphaBlendOp));

			std::cout << std::format("{}flags : 0x{:08X}\n", prefix, createInfo.pColorBlendState->pAttachments[i].colorWriteMask);

			if (i == createInfo.pColorBlendState->attachmentCount - 1)
				std::cout << "          }\n";
			else
				std::cout << "          },\n";
		}
	}

	void PipelineConfigurator::print_pDynamicState() const
	{
		const char* prefix = "        ";
		std::cout << std::format("    pDynamicState :\n");
		std::cout << "      {\n";

		std::cout << std::format("{}sType : {}\n", prefix, magic_enum::enum_name(createInfo.pDynamicState->sType));
		std::cout << std::format("{}pNext: {}\n", prefix, createInfo.pDynamicState->pNext);
		std::cout << std::format("{}flags : 0x{:08X}\n", prefix, createInfo.pDynamicState->flags);
		std::cout << std::format("{}dynamicStateCount : {}\n", prefix, createInfo.pDynamicState->dynamicStateCount);
		std::cout << "      }\n";
	}

	void PipelineConfigurator::print_pDynamicStates() const
	{
		const char* prefix = "            ";
		std::cout << std::format("        pDynamicStates[{}] :\n", createInfo.pDynamicState->dynamicStateCount);
		std::cout << "          {\n";
		for (uint32_t i = 0; i < createInfo.pDynamicState->dynamicStateCount; i++) {
				std::cout << std::format("{}pDynamicStates[{}] : {}\n", prefix, i, magic_enum::enum_name(createInfo.pDynamicState->pDynamicStates[i]));
		}
		std::cout << "          }\n";
	}

#pragma endregion

#pragma region PipelineManager

	std::pair<int, std::span<ShaderModule>> PipelineManager::RecreateShaderModule(int id, std::string& file_path)
	{
		if (auto it = mShaderModules.find(id); it != mShaderModules.end()) {
			it->second.~ShaderModule();
			it->second.Create(file_path.c_str());
			return { id, std::span<ShaderModule>(&it->second, 1) };
		}
		std::cerr << std::format("[ERROR] PipelineManager: ShaderModule with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<ShaderModule>> PipelineManager::RecreateShaderModule(int id, size_t codeSize, const uint32_t* pCode)
	{
		if (auto it = mShaderModules.find(id); it != mShaderModules.end()) {
			it->second.~ShaderModule();
			it->second.Create(codeSize, pCode);
			return { id, std::span<ShaderModule>(&it->second, 1) };
		}
		std::cerr << std::format("[ERROR] PipelineManager: ShaderModule with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<PipelineLayout>> PipelineManager::RecreatePipelineLayout(int id, VkPipelineLayoutCreateInfo& createInfo)
	{
		if (auto it = mPipelineLayouts.find(id); it != mPipelineLayouts.end()) {
			it->second.~PipelineLayout();
			it->second.Create(createInfo);
			return { id, std::span<PipelineLayout>(&it->second, 1) };
		}
		std::cerr << std::format("[ERROR] PipelineManager: PipelineLayout with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<Pipeline>> PipelineManager::RecreatePipeline(int id, VkGraphicsPipelineCreateInfo& createInfo)
	{
		if (auto it = mPipelines.find(id); it != mPipelines.end()) {
			it->second.~Pipeline();
			it->second.Create(createInfo);
			return { id, std::span<Pipeline>(&it->second, 1) };
		}
		std::cerr << std::format("[ERROR] PipelineManager: Pipeline with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<Pipeline>> PipelineManager::RecreatePipeline(int id, VkComputePipelineCreateInfo& createInfo)
	{
		if (auto it = mPipelines.find(id); it != mPipelines.end()) {
			it->second.~Pipeline();
			it->second.Create(createInfo);
			return { id, std::span<Pipeline>(&it->second, 1) };
		}
		std::cerr << std::format("[ERROR] PipelineManager: Pipeline with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<ShaderModule>> PipelineManager::CreateShaderModule(std::string name, std::string file_path)
	{
		if (auto it = mShaderModuleIDs.find(name); it != mShaderModuleIDs.end())
			return RecreateShaderModule(it->second, file_path);

		const int id = m_shader_module_id++;

		auto [it1, ok1] = mShaderModules.emplace(id, file_path.c_str());
		if (!ok1) {
			std::cerr << std::format("[ERROR] PipelineManager: Emplace shaderModule for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mShaderModuleIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] PipelineManager: ShaderModule '{}' has not been recorded!\n", name);
			mShaderModules.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<ShaderModule>(&vec, 1) };
	}

	std::pair<int, std::span<ShaderModule>> PipelineManager::CreateShaderModule(std::string name, size_t codeSize, const uint32_t* pCode)
	{
		if (auto it = mShaderModuleIDs.find(name); it != mShaderModuleIDs.end())
			return RecreateShaderModule(it->second, codeSize, pCode);

		const int id = m_shader_module_id++;

		ShaderModule shaderModule(codeSize, pCode);
		auto [it1, ok1] = mShaderModules.emplace(id, std::move(shaderModule));
		if (!ok1) {
			std::cerr << std::format("[ERROR] PipelineManager: Emplace shaderModule for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mShaderModuleIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] PipelineManager: ShaderModule '{}' has not been recorded!\n", name);
			mShaderModules.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<ShaderModule>(&vec, 1) };
	}

	std::pair<int, std::span<ShaderModule>> PipelineManager::GetShaderModule(std::string name)
	{
		if (auto it = mShaderModuleIDs.find(name); it != mShaderModuleIDs.end())
			return GetShaderModule(it->second);
		std::cerr << std::format("[ERROR] PipelineManager: ShaderModule with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<ShaderModule>> PipelineManager::GetShaderModule(int id)
	{
		if (auto it = mShaderModules.find(id); it != mShaderModules.end())
			return { id, std::span<ShaderModule>(&it->second, 1) };
		std::cerr << std::format("[ERROR] PipelineManager: ShaderModule with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	bool PipelineManager::HasShaderModule(std::string name)
	{
		if (auto it = mShaderModuleIDs.find(name); it != mShaderModuleIDs.end())
			return HasShaderModule(it->second);
		return false;
	}

	bool PipelineManager::HasShaderModule(int id)
	{
		return mShaderModules.contains(id);
	}

	size_t PipelineManager::GetShaderModuleCount() const
	{
		return mShaderModules.size();
	}

	std::pair<int, std::span<PipelineLayout>> PipelineManager::CreatePipelineLayout(std::string name, VkPipelineLayoutCreateInfo& createInfo)
	{
		if (auto it = mPipelineLayoutIDs.find(name); it != mPipelineLayoutIDs.end())
			return RecreatePipelineLayout(it->second, createInfo);

		const int id = m_shader_module_id++;

		auto [it1, ok1] = mPipelineLayouts.emplace(id, createInfo);
		if (!ok1) {
			std::cerr << std::format("[ERROR] PipelineManager: Emplace pipelineLayout for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mPipelineLayoutIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] PipelineManager: PipelineLayout '{}' has not been recorded!\n", name);
			mPipelineLayouts.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<PipelineLayout>(&vec, 1) };
	}

	std::pair<int, std::span<PipelineLayout>> PipelineManager::GetPipelineLayout(std::string name)
	{
		if (auto it = mPipelineLayoutIDs.find(name); it != mPipelineLayoutIDs.end())
			return GetPipelineLayout(it->second);
		std::cerr << std::format("[ERROR] PipelineManager: PipelineLayout with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<PipelineLayout>> PipelineManager::GetPipelineLayout(int id)
	{
		if (auto it = mPipelineLayouts.find(id); it != mPipelineLayouts.end())
			return { id, std::span<PipelineLayout>(&it->second, 1) };
		std::cerr << std::format("[ERROR] PipelineManager: PipelineLayout with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	bool PipelineManager::HasPipelineLayout(std::string name)
	{
		if (auto it = mPipelineLayoutIDs.find(name); it != mPipelineLayoutIDs.end())
			return HasPipelineLayout(it->second);
		return false;
	}

	bool PipelineManager::HasPipelineLayout(int id)
	{
		return mPipelineLayouts.contains(id);
	}

	size_t PipelineManager::GetPipelineLayoutCount() const
	{
		return mPipelineLayouts.size();
	}

	std::pair<int, std::span<Pipeline>> PipelineManager::CreatePipeline(std::string name, VkGraphicsPipelineCreateInfo& createInfo)
	{
		if (auto it = mPipelineIDs.find(name); it != mPipelineIDs.end())
			return RecreatePipeline(it->second, createInfo);

		const int id = m_shader_module_id++;

		auto [it1, ok1] = mPipelines.emplace(id, createInfo);
		if (!ok1) {
			std::cerr << std::format("[ERROR] PipelineManager: Emplace pipeline for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mPipelineIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] PipelineManager: Pipeline '{}' has not been recorded!\n", name);
			mPipelines.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<Pipeline>(&vec, 1) };
	}

	std::pair<int, std::span<Pipeline>> PipelineManager::CreatePipeline(std::string name, VkComputePipelineCreateInfo& createInfo)
	{
		if (auto it = mPipelineIDs.find(name); it != mPipelineIDs.end())
			return RecreatePipeline(it->second, createInfo);

		const int id = m_shader_module_id++;

		auto [it1, ok1] = mPipelines.emplace(id, createInfo);
		if (!ok1) {
			std::cerr << std::format("[ERROR] PipelineManager: Emplace pipeline for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mPipelineIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] PipelineManager: Pipeline '{}' has not been recorded!\n", name);
			mPipelines.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<Pipeline>(&vec, 1) };
	}

	std::pair<int, std::span<Pipeline>> PipelineManager::GetPipeline(std::string name)
	{
		if (auto it = mPipelineIDs.find(name); it != mPipelineIDs.end())
			return GetPipeline(it->second);
		std::cerr << std::format("[ERROR] PipelineManager: Pipeline with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<Pipeline>> PipelineManager::GetPipeline(int id)
	{
		if (auto it = mPipelines.find(id); it != mPipelines.end())
			return { id, std::span<Pipeline>(&it->second, 1) };
		std::cerr << std::format("[ERROR] PipelineManager: Pipeline with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	int PipelineManager::DestroyPipeline(std::string name)
	{
		if (auto it = mPipelineIDs.find(name); it != mPipelineIDs.end())
			return DestroyPipeline(it->second);
		std::cerr << std::format("[WARNING] PipelineManager: Pipelines with name '{}' do not exist!\n", name);
		return M_INVALID_ID;
	}

	int PipelineManager::DestroyPipeline(int id)
	{
		if (auto it = mPipelines.find(id); it != mPipelines.end()) {
			it->second.~Pipeline();
			return it->first;
		}
		std::cerr << std::format("[WARNING] PipelineManager: Pipelines with id {} do not exist!\n", id);
		return M_INVALID_ID;
	}

	bool PipelineManager::HasPipeline(std::string name)
	{
		if (auto it = mPipelineIDs.find(name); it != mPipelineIDs.end())
			return HasPipeline(it->second);
		return false;
	}

	bool PipelineManager::HasPipeline(int id)
	{
		return mPipelines.contains(id);
	}

	size_t PipelineManager::GetPipelineCount() const
	{
		return mPipelines.size();
	}

#pragma endregion

}