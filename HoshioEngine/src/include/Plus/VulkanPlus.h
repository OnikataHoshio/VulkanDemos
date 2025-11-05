#ifndef _VULKAN_PLUS_H_
#define _VULKAN_PLUS_H_

#include "Base/CommandManager.h"
#include "Base/RpwfManager.h"
#include "Base/DescriptorManager.h"
#include "Base/QueryPoolManager.h"
#include "Base/SamplerManager.h"
#include "Base/SyncManager.h"
#include "Base/PipelineManager.h"
#include "Plus/ImageManager.h"

namespace HoshioEngine {
	
	const uint32_t FORMAT_COUNT = 184;

	struct DefaultVertex {
		glm::vec2 position;
		glm::vec2 texCoord;
	};

	class VulkanPlus {
	private:
		VkFormatProperties formatProperties[FORMAT_COUNT] = {};
		CommandPool commandPool_graphics;
		CommandPool commandPool_compute;
		CommandBuffer commandBuffer_graphics;
		CommandBuffer commandBuffer_present;

		DescriptorPool descriptorPool;
		std::vector<Framebuffer> swapchainFramebuffers;
		RenderPass swapchainRenderPass;
		VertexBuffer defaultVertexBuffer;

		ImageManager image_manager;
		SamplerManager sampler_manager;
		RpwfManager rpwf_manager;
		QueryPoolManager query_pool_manager;
		PipelineManager pipeline_manager;
		DescriptorManager descriptor_manager;
		SyncManager sync_manager;


		VulkanPlus();
		~VulkanPlus() = default;
		VulkanPlus(VulkanPlus&& other) = delete;
		VulkanPlus(const VulkanPlus& other) = delete;
		VulkanPlus& operator=(const VulkanPlus& other) = delete;

		void InitSwapchainRenderPass();
		void InitSwapchainFramebuffers();

	public:
		static VulkanPlus& Plus();

		const VkFormatProperties& FormatProperties(VkFormat format) const;
		const CommandPool& CommandPool_Graphics() const;
		const CommandPool& CommandPool_Compute() const;
		const CommandBuffer& CommandBuffer_Graphics() const;
		const CommandBuffer& CommandBuffer_Present() const;
		const CommandBuffer& CommandBuffer_Transfer() const;
		const DescriptorPool& DescriptorPool() const;
		const VertexBuffer& DefaultVertexBuffer() const;
		const Framebuffer& CurrentSwapchainFramebuffer() const;
		const Framebuffer& SwapchainFramebuffer() const;
		const RenderPass& SwapchainRenderPass() const;

		void ExecuteCommandBuffer_Graphics(VkCommandBuffer commandBuffer) const;

		std::pair<int, std::span<Texture2D>> CreateTexture2D(std::string name, const char* filePath, VkFormat initial_format, VkFormat final_format, bool generateMip = true);
		std::pair<int, std::span<Texture2D>> CreateTexture2D(std::string name, const uint8_t* pImageData, VkExtent2D extent, VkFormat initial_format, VkFormat final_format, bool generateMip = true);
		std::pair<int, std::span<Texture2D>> GetTexture2D(std::string name);
		std::pair<int, std::span<Texture2D>> GetTexture2D(int id);
		bool HasTexture2D(std::string name);
		bool HasTexture2D(int id);
		size_t GetTextureCount() const;

		std::pair<int, std::span<ColorAttachment>> CreateColorAttachments(std::string name, uint32_t count, VkFormat format, VkExtent2D extent, bool hasMipmap = true, uint32_t layerCount = 1,
			VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags otherUsages = 0);
		int DestroyColorAttachments(std::string name);
		int DestroyColorAttachments(int id);
		std::pair<int, std::span<ColorAttachment>> GetColorAttachments(std::string name);
		std::pair<int, std::span<ColorAttachment>> GetColorAttachments(int id);
		bool HasColorAttachments(std::string name);
		bool HasColorAttachments(int id);
		size_t GetColorAttachmentsCount() const;

		std::pair<int, std::span<DepthStencilAttachment>> CreateDepthStencilAttachments(std::string name, uint32_t count, VkFormat format, VkExtent2D extent, bool stencilOnly = false, uint32_t layerCount = 1,
			VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags otherUsages = 0);
		int DestroyDepthStencilAttachments(std::string name);
		int DestroyDepthStencilAttachments(int id);
		std::pair<int, std::span<DepthStencilAttachment>> GetDepthStencilAttachments(std::string name);
		std::pair<int, std::span<DepthStencilAttachment>> GetDepthStencilAttachments(int id);
		bool HasDepthStencilAttachments(std::string name);
		bool HasDepthStencilAttachments(int id);
		size_t GetDepthStencilAttachmentsCount() const;

 		std::pair<int, std::span<Framebuffer>> CreateFramebuffers(std::string name, uint32_t count, std::vector<VkFramebufferCreateInfo>& createInfos);
		std::pair<int, std::span<Framebuffer>> GetFramebuffers(std::string name);
		std::pair<int, std::span<Framebuffer>> GetFramebuffers(int id);
		int DestroyFramebuffers(std::string name);
		int DestroyFramebuffers(int id);
		bool HasFramebuffers(std::string name);
		bool HasFramebuffers(int id);
		size_t GetFramebuffersCount() const;

		std::pair<int, std::span<Fence>> CreateFences(std::string name, uint32_t count, VkFenceCreateFlags flags = 0);
		std::pair<int, std::span<Fence>> GetFences(std::string name);
		std::pair<int, std::span<Fence>> GetFences(int id);
		bool HasFence(std::string name);
		bool HasFence(int id);
		size_t GetFencesCount() const;

		std::pair<int, std::span<Semaphore>> CreateSemaphores(std::string name, uint32_t count, VkSemaphoreCreateFlags flags = 0);
		std::pair<int, std::span<Semaphore>> GetSemaphores(std::string name);
		std::pair<int, std::span<Semaphore>> GetSemaphores(int id);
		bool HasSemaphore(std::string name);
		bool HasSemaphore(int id);
		size_t GetSemaphoresCount() const;

		std::pair<int, std::span<ShaderModule>>  CreateShaderModule(std::string name, const char* filePath);
		std::pair<int, std::span<ShaderModule>>  CreateShaderModule(std::string name, size_t codeSize, const uint32_t* pCode);
		std::pair<int, std::span<ShaderModule>>  GetShaderModule(std::string name);
		std::pair<int, std::span<ShaderModule>>  GetShaderModule(int id);
		bool HasShaderModule(std::string name);
		bool HasShaderModule(int id);
		size_t GetShaderModuleCount() const;

		std::pair<int, std::span<Pipeline>> CreatePipeline(std::string name, VkGraphicsPipelineCreateInfo& createInfo);
		std::pair<int, std::span<Pipeline>> CreatePipeline(std::string name, VkComputePipelineCreateInfo& createInfo);
		bool HasPipeline(std::string name);
		bool HasPipeline(int id);
		int DestroyPipeline(std::string name);
		int DestroyPipeline(int id);
		std::pair<int, std::span<Pipeline>> GetPipeline(std::string name);
		std::pair<int, std::span<Pipeline>> GetPipeline(int id);
		size_t GetPipelineCount() const;

		std::pair<int, std::span<PipelineLayout>> CreatePipelineLayout(std::string name, VkPipelineLayoutCreateInfo& createInfo);
		std::pair<int, std::span<PipelineLayout>> GetPipelineLayout(std::string name);
		std::pair<int, std::span<PipelineLayout>> GetPipelineLayout(int id);
		bool HasPipelineLayout(std::string name);
		bool HasPipelineLayout(int id);
		size_t GetPipelineLayoutCount() const;

		std::pair<int, std::span<RenderPass>> CreateRenderPass(std::string name, VkRenderPassCreateInfo& createInfo);
		std::pair<int, std::span<RenderPass>> GetRenderPass(std::string name);
		std::pair<int, std::span<RenderPass>> GetRenderPass(int id);
		bool HasRenderPass(std::string name);
		bool HasRenderPass(int id);
		size_t GetRenderPassCount() const;

		std::pair<int, std::span<TimestampQueries>> CreateTimestampQueries(std::string name, uint32_t count, uint32_t capacity);
		std::pair<int, std::span<TimestampQueries>> GetTimestampQueries(std::string name);
		std::pair<int, std::span<TimestampQueries>> GetTimestampQueries(int id);
		bool HasTimestampQueries(std::string name);
		bool HasTimestampQueries(int id);
		size_t GetTimestampQueriesCount() const;

		std::pair<int, std::span<Sampler>> CreateSampler(std::string name, VkSamplerCreateInfo& createInfo);
		std::pair<int, std::span<Sampler>> GetSampler(std::string name);
		std::pair<int, std::span<Sampler>> GetSampler(int id);
		bool HasSampler(std::string name);
		bool HasSampler(int id);
		size_t GetSamplerCount() const;

		std::pair<int, std::span<DescriptorSetLayout>> CreateDescriptorSetLayout(std::string name, VkDescriptorSetLayoutCreateInfo& createInfo);
		std::pair<int, std::span<DescriptorSetLayout>> GetDescriptorSetLayout(std::string name);
		std::pair<int, std::span<DescriptorSetLayout>> GetDescriptorSetLayout(int id);
		bool HasDescriptorSetLayout(std::string name);
		bool HasDescriptorSetLayout(int id);
		size_t GetDescriptorSetLayoutCount() const;
	};

}


#endif // !_VULKAN_PlusER_H_

