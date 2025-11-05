#ifndef _PIPELINE_MANAGER_H_
#define _PIPELINE_MANAGER_H_

#include "Base/VulkanBase.h"

namespace HoshioEngine {
	class ShaderModule {
	private:
		VkShaderModule handle = VK_NULL_HANDLE;
	public:
		ShaderModule() = default;
		ShaderModule(VkShaderModuleCreateInfo& createInfo);
		ShaderModule(const char* filePath);
		ShaderModule(size_t codeSize, const uint32_t* pCode);
		ShaderModule(ShaderModule&& other) noexcept;
		~ShaderModule();
		
		operator VkShaderModule() const;
		const VkShaderModule* Address() const;

		VkPipelineShaderStageCreateInfo ShaderStageCi(VkShaderStageFlagBits stage, const char* entry = "main") const;

		void Create(VkShaderModuleCreateInfo& createInfo);
		void Create(const char* filePath);
		void Create(size_t codeSize, const uint32_t* pCode);
	};

	class PipelineLayout {
	private:
		VkPipelineLayout handle = VK_NULL_HANDLE;
	public:
		PipelineLayout() = default;
		PipelineLayout(VkPipelineLayoutCreateInfo& createInfo);
		PipelineLayout(PipelineLayout&& other);
		~PipelineLayout();
		operator VkPipelineLayout() const;
		const VkPipelineLayout* Address() const;

		void Create(VkPipelineLayoutCreateInfo& createInfo);

	};

	class Pipeline {
	private:
		VkPipeline handle = VK_NULL_HANDLE;
	public:
		Pipeline() = default;
		Pipeline(VkGraphicsPipelineCreateInfo& createInfo);
		Pipeline(VkComputePipelineCreateInfo& createInfo);
		Pipeline(Pipeline&& other);
		~Pipeline();
		operator VkPipeline() const;
		const VkPipeline* Address() const;

		void Create(VkGraphicsPipelineCreateInfo& createInfo);

		void Create(VkComputePipelineCreateInfo& createInfo);
	};

	class PipelineConfigurator {
	private:
		VkGraphicsPipelineCreateInfo createInfo = {};

		//Shader
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

		//VertexInput
		VkPipelineVertexInputStateCreateInfo vertexInputStateCi = {};
		std::vector<VkVertexInputBindingDescription> vertexInputBindings;
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes;

		//InputAssembly
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCi = {};

		//Tessellation
		VkPipelineTessellationStateCreateInfo tessellationStateCi = {};

		//Viewport
		VkPipelineViewportStateCreateInfo viewportStateCi = {};
		std::vector<VkViewport> viewports;
		std::vector<VkRect2D> scissors;

		//Rasterization
		VkPipelineRasterizationStateCreateInfo rasterizationStateCi = {};

		//Multisample
		VkPipelineMultisampleStateCreateInfo multisampleStateCi = {};

		//DepthStencil
		VkPipelineDepthStencilStateCreateInfo depthStencilStateCi = {};

		//ColorBlend
		VkPipelineColorBlendStateCreateInfo colorBlendStateCi = {};
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates;

		//Dynamic
		VkPipelineDynamicStateCreateInfo dynamicStateCi = {};
		std::vector<VkDynamicState> dynamicStates;

		void print_pStages() const;

		void print_pVertexInputState() const;

		void print_pVertexBindingDescriptions() const;

		void print_pVertexAttributeDescriptions() const;

		void print_pInputAssemblyState() const;

		void print_pTessellationState() const;

		void print_pViewportState() const;

		void print_pViewports() const;

		void print_pScissors() const;

		void print_pRasterizationState() const;

		void print_pMultisampleState() const;

		void print_pDepthStencilState() const;

		void print_pColorBlendState() const;

		void print_pAttachments() const;

		void print_pDynamicState() const;

		void print_pDynamicStates() const;

	public:
		PipelineConfigurator();
		PipelineConfigurator(const PipelineConfigurator& other) noexcept;
		~PipelineConfigurator() = default;
		operator VkGraphicsPipelineCreateInfo() const;

		operator VkGraphicsPipelineCreateInfo&();

		void PrintPipelineCreateInfo(const char* tip = "PipelineCreateInfo:") const;

		PipelineConfigurator& PipelineLayout(VkPipelineLayout pipelineLayout);

		PipelineConfigurator& RenderPass(VkRenderPass renderPass);

		PipelineConfigurator& Subpass(uint32_t subpass_index);

		PipelineConfigurator& BasePipelineHandle(VkPipeline basePipeline);

		PipelineConfigurator& BasePipelineIndex(int32_t basePipelineIndex);

		PipelineConfigurator& AddShaderStage(VkPipelineShaderStageCreateInfo shaderStage);

		PipelineConfigurator& AddVertexInputBindings(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate = VK_VERTEX_INPUT_RATE_VERTEX);

		PipelineConfigurator& AddVertexInputAttribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset);

		PipelineConfigurator& PrimitiveTopology(VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable = VK_FALSE);

		PipelineConfigurator& TessPatchControlPoints(uint32_t controlPointCount);

		PipelineConfigurator& AddViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 0.0f);

		PipelineConfigurator& AddScissor(VkOffset2D offset, VkExtent2D extent);

		PipelineConfigurator& EnableDepthClamp(VkBool32 depthClampEnable);

		PipelineConfigurator& EnableRasterizerDiscard(VkBool32 discardRasterizer);

		PipelineConfigurator& PolygonMode(VkPolygonMode polygonMode);

		PipelineConfigurator& CullMode(VkCullModeFlags cullMode);

		PipelineConfigurator& FrontFace(VkFrontFace frontFace);

		PipelineConfigurator& LineWidth(float lineWidth);

		PipelineConfigurator& EnableDepthBias(VkBool32 depthBiasEnable, float depthBiasConstantFactor = 0.0f, float depthBiasClamp = 0.0f, float depthBiasSlopeFactor = 0.0f);

		PipelineConfigurator& RasterizationSamples(VkSampleCountFlagBits rasterizationSamples);

		PipelineConfigurator& EnableSampleShading(VkBool32 sampleShadingEnable);

		PipelineConfigurator& EnableDepthTest(VkBool32 depthTestEnable, VkBool32 depthWriteEnable = VK_FALSE, VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS);

		PipelineConfigurator& EnableDepthBoundsTest(VkBool32 depthBoundsTestEnable, float minDepthBounds, float maxDepthBounds);

		PipelineConfigurator& EnableStencilTest(VkBool32 stencilTestEnable, VkStencilOpState& front, VkStencilOpState& back);

		PipelineConfigurator& EnableLogicOp(VkBool32 logicOpEnable, VkLogicOp logicOp = VK_LOGIC_OP_CLEAR);

		PipelineConfigurator& AddAttachmentState(VkColorComponentFlags colorWriteMask);
		
		PipelineConfigurator& EnableBlend(VkBool32 blendEnable, uint32_t attachmentIndex);

		PipelineConfigurator& EnableBlend(VkBool32 blendEnable);
		
		PipelineConfigurator& ColorBlend(uint32_t attachmentIndex, VkBlendFactor srcColorBlendFactor, VkBlendFactor dstColorBlendFactor, VkBlendOp colorBlendOp);
		
		PipelineConfigurator& ColorBlend(VkBlendFactor srcColorBlendFactor, VkBlendFactor dstColorBlendFactor, VkBlendOp colorBlendOp);

		PipelineConfigurator& AlphaBlend(uint32_t attachmentIndex, VkBlendFactor srcAlphaBlendFactor, VkBlendFactor dstAlphaBlendFactor, VkBlendOp alphaBlendOp);

		PipelineConfigurator& AlphaBlend(VkBlendFactor srcAlphaBlendFactor, VkBlendFactor dstAlphaBlendFactor, VkBlendOp alphaBlendOp);

		PipelineConfigurator& AddDynamicState(VkDynamicState dynamicState);

		PipelineConfigurator& UpdatePipelineCreateInfo();

	private:
		void BindFixFunctionCi();

		void UpdateCiInnerPointer();
	};

	class PipelineManager {
	private:
		int m_shader_module_id = 0;
		int m_pipeline_layout_id = 0;
		int m_pipeline_id = 0;

		std::unordered_map<std::string, int> mShaderModuleIDs;
		std::unordered_map<int, ShaderModule> mShaderModules;
		std::unordered_map<std::string, int> mPipelineLayoutIDs;
		std::unordered_map<int, PipelineLayout> mPipelineLayouts;
		std::unordered_map<std::string, int> mPipelineIDs;
		std::unordered_map<int, Pipeline> mPipelines;

		std::pair<int, std::span<ShaderModule>> RecreateShaderModule(int id, std::string& file_path);
		std::pair<int, std::span<ShaderModule>> RecreateShaderModule(int id, size_t codeSize, const uint32_t* pCode);
		std::pair<int, std::span<PipelineLayout>> RecreatePipelineLayout(int id, VkPipelineLayoutCreateInfo& createInfo);
		std::pair<int, std::span<Pipeline>> RecreatePipeline(int id, VkGraphicsPipelineCreateInfo& createInfo);
		std::pair<int, std::span<Pipeline>> RecreatePipeline(int id, VkComputePipelineCreateInfo& createInfo);

	public:
		std::pair<int, std::span<ShaderModule>> CreateShaderModule(std::string name, std::string file_path);
		std::pair<int, std::span<ShaderModule>> CreateShaderModule(std::string name, size_t codeSize, const uint32_t* pCode);
		std::pair<int, std::span<ShaderModule>> GetShaderModule(std::string name);
		std::pair<int, std::span<ShaderModule>> GetShaderModule(int id);
		bool HasShaderModule(std::string name);
		bool HasShaderModule(int id);
		size_t GetShaderModuleCount() const;

		std::pair<int, std::span<PipelineLayout>> CreatePipelineLayout(std::string name, VkPipelineLayoutCreateInfo& createInfo);
		std::pair<int, std::span<PipelineLayout>> GetPipelineLayout(std::string name);
		std::pair<int, std::span<PipelineLayout>> GetPipelineLayout(int id);
		bool HasPipelineLayout(std::string name);
		bool HasPipelineLayout(int id);
		size_t GetPipelineLayoutCount() const;

		std::pair<int, std::span<Pipeline>> CreatePipeline(std::string name, VkGraphicsPipelineCreateInfo& createInfo);
		std::pair<int, std::span<Pipeline>> CreatePipeline(std::string name, VkComputePipelineCreateInfo& createInfo);
		std::pair<int, std::span<Pipeline>> GetPipeline(std::string name);
		std::pair<int, std::span<Pipeline>> GetPipeline(int id);
		int DestroyPipeline(std::string name);
		int DestroyPipeline(int id);
		bool HasPipeline(std::string name);
		bool HasPipeline(int id);
		size_t GetPipelineCount() const;
	};
}

#endif // !_PIPELINE_MANAGER_H_

