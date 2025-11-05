#include "Base/RpwfManager.h"

namespace HoshioEngine {
#pragma region RenderPass

	RenderPass::RenderPass(VkRenderPassCreateInfo& createInfo)
	{
		Create(createInfo);
	}

	RenderPass::RenderPass(RenderPass&& other) noexcept
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}

	RenderPass::~RenderPass()
	{
		if (handle) {
			vkDestroyRenderPass(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}

	RenderPass::operator VkRenderPass() const
	{
		return handle;
	}

	const VkRenderPass* RenderPass::Address() const
	{
		return &handle;
	}

	void RenderPass::Begin(VkCommandBuffer commandBuffer, VkRenderPassBeginInfo& beginInfo, VkSubpassContents subpassContents) const
	{
		beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		beginInfo.renderPass = handle;
		vkCmdBeginRenderPass(commandBuffer, &beginInfo, subpassContents);
	}

	void RenderPass::Begin(VkCommandBuffer commandBuffer, VkFramebuffer framebuffer, VkRect2D renderArea, ArrayRef<const VkClearValue> clearValues, VkSubpassContents subpassContents) const
	{
		VkRenderPassBeginInfo beginInfo = {
			.framebuffer = framebuffer,
			.renderArea = renderArea,
			.clearValueCount = static_cast<uint32_t>(clearValues.size()),
			.pClearValues = clearValues.data()
		};
		Begin(commandBuffer, beginInfo, subpassContents);
	}

	void RenderPass::NextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents subpassContents) const
	{
		vkCmdNextSubpass(commandBuffer, subpassContents);
	}

	void RenderPass::End(VkCommandBuffer commandBuffer) const
	{
		vkCmdEndRenderPass(commandBuffer);
	}

	void RenderPass::Create(VkRenderPassCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		if (vkCreateRenderPass(VulkanBase::Base().Device(), &createInfo, nullptr, &handle))
			throw std::runtime_error("Failed to create a render pass");
	}

#pragma endregion

#pragma region Framebuffer

	Framebuffer::Framebuffer(VkFramebufferCreateInfo& createInfo)
	{
		Create(createInfo);
	}

	Framebuffer::Framebuffer(Framebuffer&& other) noexcept
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}

	Framebuffer::~Framebuffer()
	{
		if (handle) {
			vkDestroyFramebuffer(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}

	Framebuffer::operator VkFramebuffer() const
	{
		return handle;
	}

	const VkFramebuffer* Framebuffer::Address() const
	{
		return &handle;
	}

	void Framebuffer::Create(VkFramebufferCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		if (vkCreateFramebuffer(VulkanBase::Base().Device(),&createInfo,nullptr,&handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a frame buffer");
	}

#pragma endregion

#pragma region RpwfManager

	std::pair<int, std::span<RenderPass>> RpwfManager::RecreateRenderPass(int id, VkRenderPassCreateInfo& createInfo)
	{
		if (auto it = mRenderPasses.find(id); it != mRenderPasses.end()) {
			it->second.~RenderPass();
			it->second.Create(createInfo);
			return { id, std::span<RenderPass>(&it->second, 1) };
		}
		std::cerr << std::format("[ERROR] RpwfManager: Renderpass with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<Framebuffer>> RpwfManager::RecreateFramebuffers(int id, uint32_t count, std::vector<VkFramebufferCreateInfo>& createInfos)
	{

		if (count == 0) {
			std::cerr << std::format("[ERROR] SyncManager: '{}' requested 0 Framebuffers\n", id);
			return { M_INVALID_ID, {} };
		}

		if (auto it = mFramebuffers.find(id); it != mFramebuffers.end()) {
			it->second.clear();
			it->second.resize(count);
			for (size_t i = 0; i < it->second.size(); i++)
				it->second[i].Create(createInfos[i]);
			return { id, std::span<Framebuffer>(it->second.data(), it->second.size()) };
		}
		std::cerr << std::format("[ERROR] SyncManager: Framebuffers with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<RenderPass>> RpwfManager::CreateRenderPass(std::string name, VkRenderPassCreateInfo& createInfo)
	{
		if (auto it = mRenderPassIDs.find(name); it != mRenderPassIDs.end())
			return RecreateRenderPass(it->second, createInfo);

		const int id = m_renderpass_id++;

		auto [it1, ok1] = mRenderPasses.emplace(id, createInfo);
		if (!ok1) {
			std::cerr << std::format("[ERROR] RpwfManager: Emplace renderpass for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mRenderPassIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] RpwfPassManager: Renderpass '{}' has not been recorded!\n", name);
			mRenderPasses.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<RenderPass>(&vec, 1) };
	}

	std::pair<int, std::span<RenderPass>> RpwfManager::GetRenderPass(std::string name)
	{
		if (auto it = mRenderPassIDs.find(name); it != mRenderPassIDs.end())
			return GetRenderPass(it->second);
		std::cerr << std::format("[ERROR] RpwfManager: RenderPass with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<RenderPass>> RpwfManager::GetRenderPass(int id)
	{
		if (auto it = mRenderPasses.find(id); it != mRenderPasses.end())
			return { id, std::span<RenderPass>(&it->second, 1) };
		std::cerr << std::format("[ERROR] RpwfManager: Renderpass with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	bool RpwfManager::HasRenderPass(std::string name)
	{
		if (auto it = mRenderPassIDs.find(name); it != mRenderPassIDs.end())
			return HasRenderPass(it->second);
		return false;
	}

	bool RpwfManager::HasRenderPass(int id)
	{
		return mRenderPasses.contains(id);
	}

	size_t RpwfManager::GetRenderPassCount() const
	{
		return mRenderPasses.size();
	}


	std::pair<int, std::span<Framebuffer>> RpwfManager::CreateFramebuffers(std::string name, uint32_t count, std::vector<VkFramebufferCreateInfo>& createInfos)
	{
		if (auto it = mFramebufferIDs.find(name); it != mFramebufferIDs.end()) 
			return RecreateFramebuffers(it->second, count, createInfos);

		if (count == 0) {
			std::cerr << std::format("[ERROR] SyncManager: '{}' requested 0 Framebuffers\n", name);
			return { M_INVALID_ID, {} };
		}

		std::vector<Framebuffer> framebuffers(count);
		for (size_t i = 0; i < framebuffers.size(); i++)
			framebuffers[i].Create(createInfos[i]);

		const int id = m_framebuffer_id++;

		auto [it1, ok1] = mFramebuffers.emplace(id, std::move(framebuffers));
		if (!ok1) {
			std::cerr << std::format("[ERROR] SyncManager: Emplace framebuffers for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mFramebufferIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] SyncManager: Framebuffer '{}' has not been recorded!\n", name);
			mFramebuffers.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<Framebuffer>(vec.data(), vec.size()) };
	}

	std::pair<int, std::span<Framebuffer>> RpwfManager::GetFramebuffers(std::string name)
	{
		if (auto it = mFramebufferIDs.find(name); it != mFramebufferIDs.end())
			return GetFramebuffers(it->second);
		std::cerr << std::format("[ERROR] SyncManager: Framebuffers with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<Framebuffer>> RpwfManager::GetFramebuffers(int id)
	{
		if (auto it = mFramebuffers.find(id); it != mFramebuffers.end())
			return { id, std::span<Framebuffer>(it->second.data(), it->second.size()) };
		std::cerr << std::format("[ERROR] SyncManager: Framebuffers with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	int RpwfManager::DestroyFramebuffers(std::string name)
	{
		if (auto it = mFramebufferIDs.find(name); it != mFramebufferIDs.end())
			return DestroyFramebuffers(it->second);
		std::cerr << std::format("[WARNING] SyncManager: Framebuffers with name '{}' do not exist!\n", name);
		return M_INVALID_ID;
	}

	int RpwfManager::DestroyFramebuffers(int id)
	{
		if (auto it = mFramebuffers.find(id); it != mFramebuffers.end()) {
			it->second.clear();
			return it->first;
		}
		std::cerr << std::format("[WARNING] SyncManager: Framebuffers with id {} do not exist!\n", id);
		return M_INVALID_ID;
	}

	bool RpwfManager::HasFramebuffer(std::string name)
	{
		if (auto it = mFramebufferIDs.find(name); it != mFramebufferIDs.end())
			return HasFramebuffer(it->second);
		return false;
	}

	bool RpwfManager::HasFramebuffer(int id)
	{
		return mFramebuffers.contains(id);
	}

	size_t RpwfManager::GetFramebuffersCount() const
	{
		return mFramebuffers.size();;
	}

#pragma endregion


}