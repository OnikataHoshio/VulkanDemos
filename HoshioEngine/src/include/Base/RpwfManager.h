#ifndef _FBO_MANAGER_H_
#define _FBO_MANAGER_H_

#include "Base/VulkanBase.h"

namespace HoshioEngine {
	class RenderPass {
	private:
		VkRenderPass handle = VK_NULL_HANDLE;
	public:
		RenderPass() = default;
		RenderPass(VkRenderPassCreateInfo& createInfo);
		RenderPass(RenderPass&& other) noexcept;
		~RenderPass();
		operator VkRenderPass() const;
		const VkRenderPass* Address() const;

		void Begin(VkCommandBuffer commandBuffer, VkRenderPassBeginInfo& beginInfo, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const;

		void Begin(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkRect2D renderArea, ArrayRef<const VkClearValue> clearValues = {}, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const;

		void NextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE) const;

		void End(VkCommandBuffer commandBuffer) const;

		void Create(VkRenderPassCreateInfo& createInfo);
	};

	class Framebuffer {
	private:
		VkFramebuffer handle = VK_NULL_HANDLE;
	public:
		Framebuffer() = default;
		Framebuffer(VkFramebufferCreateInfo& createInfo);
		Framebuffer(Framebuffer&& other) noexcept;
		~Framebuffer();
		operator VkFramebuffer() const;
		const VkFramebuffer* Address() const;

		void Create(VkFramebufferCreateInfo& createInfo);
	};

	class RpwfManager {
	private:
		int m_renderpass_id = 0;
		int m_framebuffer_id = 0;

		std::unordered_map<std::string, int> mRenderPassIDs;
		std::unordered_map<int, RenderPass> mRenderPasses;
		std::unordered_map<std::string, int> mFramebufferIDs;
		std::unordered_map<int, std::vector<Framebuffer>> mFramebuffers;

		std::pair<int, std::span<RenderPass>> RecreateRenderPass(int id, VkRenderPassCreateInfo& createInfo);
		std::pair<int, std::span<Framebuffer>> RecreateFramebuffers(int id, uint32_t count, std::vector<VkFramebufferCreateInfo>& createInfos);

	public:
		std::pair<int, std::span<RenderPass>> CreateRenderPass(std::string name, VkRenderPassCreateInfo& createInfo);
		std::pair<int, std::span<RenderPass>> GetRenderPass(std::string name);
		std::pair<int, std::span<RenderPass>> GetRenderPass(int id);
		bool HasRenderPass(std::string name);
		bool HasRenderPass(int id);
		size_t GetRenderPassCount() const;


		std::pair<int, std::span<Framebuffer>> CreateFramebuffers(std::string name, uint32_t count, std::vector<VkFramebufferCreateInfo>& createInfos);
		std::pair<int, std::span<Framebuffer>> GetFramebuffers(std::string name);
		std::pair<int, std::span<Framebuffer>> GetFramebuffers(int id);
		int DestroyFramebuffers(std::string name);
		int DestroyFramebuffers(int id);
		bool HasFramebuffer(std::string name);
		bool HasFramebuffer(int id);
		size_t GetFramebuffersCount() const;
	};

	
}

#endif // !_RPWF_MANAGER_H_

