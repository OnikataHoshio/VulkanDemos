#include "Base/SyncManager.h"

namespace HoshioEngine {

#pragma region Fence

	Fence::Fence(VkFenceCreateFlags flags)
	{
		Create(flags);
	}
	Fence::Fence(VkFenceCreateInfo& createInfo)
	{
		Create(createInfo);
	}
	Fence::Fence(Fence&& other) noexcept
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}
	Fence::~Fence()
	{
		if (handle) {
			vkDestroyFence(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}
	Fence::operator VkFence() const
	{
		return handle;
	}
	const VkFence* Fence::Address() const
	{
		return &handle;
	}
	void Fence::Wait() const
	{
		if (vkWaitForFences(VulkanBase::Base().Device(), 1, &handle, false, UINT64_MAX) != VK_SUCCESS)
			throw std::runtime_error("Failed to wait for fence");
	}
	void Fence::Reset() const
	{
		if (vkResetFences(VulkanBase::Base().Device(), 1, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to reset fence");
	}
	void Fence::WaitAndReset() const
	{
		Wait();
		Reset();
	}
	void Fence::Status() const
	{
		if (vkGetFenceStatus(VulkanBase::Base().Device(), handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to get fence status");
	}
	void Fence::Create(VkFenceCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		if (vkCreateFence(VulkanBase::Base().Device(), &createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a fence");
	}
	void Fence::Create(VkFenceCreateFlags flags)
	{
		VkFenceCreateInfo createInfo = {
			.flags = flags
		};
		Create(createInfo);
	}
#pragma endregion

#pragma region Semaphore

	Semaphore::Semaphore(VkSemaphoreCreateFlags flags)
	{
		Create(flags);
	}
	Semaphore::Semaphore(VkSemaphoreCreateInfo& createInfo)
	{
		Create(createInfo);
	}
	Semaphore::Semaphore(Semaphore&& other) noexcept
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}
	Semaphore::~Semaphore()
	{
		if (handle) {
			vkDestroySemaphore(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}
	Semaphore::operator VkSemaphore() const
	{
		return handle;
	}
	const VkSemaphore* Semaphore::Address() const
	{
		return &handle;
	}
	void Semaphore::Create(VkSemaphoreCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		if (vkCreateSemaphore(VulkanBase::Base().Device(), &createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a semaphore");
	}
	void Semaphore::Create(VkSemaphoreCreateFlags flags)
	{
		VkSemaphoreCreateInfo createInfo = {
			.flags = flags
		};
		Create(createInfo);
	}

#pragma endregion

#pragma region SyncManager

	std::pair<int, std::span<Fence>> SyncManager::RecreateFences(int id, uint32_t count, VkFenceCreateFlags flags)
	{
		if (count == 0) {
			std::cerr << std::format("[ERROR] SyncManager: '{}' requested 0 Fences\n", id);
			return { M_INVALID_ID, {} };
		}

		if (auto it = mFences.find(id); it != mFences.end()) {
			it->second.clear();
			it->second.reserve(count);
			for (size_t i = 0; i < count; i++)
				it->second.emplace_back(flags);
			return { id, std::span<Fence>(it->second.data(), it->second.size()) };
		}
		std::cerr << std::format("[ERROR] SyncManager: Fences with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<Semaphore>> SyncManager::RecreateSemaphores(int id, uint32_t count, VkSemaphoreCreateFlags flags)
	{
		if (count == 0) {
			std::cerr << std::format("[ERROR] SyncManager: '{}' requested 0 Semaphores\n", id);
			return { M_INVALID_ID, {} };
		}

		if (auto it = mSemaphores.find(id); it != mSemaphores.end()) {
			it->second.clear();
			it->second.reserve(count);
			for (size_t i = 0; i < count; i++)
				it->second.emplace_back(flags);
			return { id, std::span<Semaphore>(it->second.data(), it->second.size()) };
		}
		std::cerr << std::format("[ERROR] SyncManager: Semaphores with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<Fence>> SyncManager::CreateFences(std::string name, uint32_t count, VkFenceCreateFlags flags)
	{
		if (auto it = mFenceIDs.find(name); it != mFenceIDs.end())
			return RecreateFences(it->second, count, flags);

		if (count == 0) {
			std::cerr << std::format("[ERROR] SyncManager: '{}' requested 0 fences\n", name);
			return { M_INVALID_ID, {} };
		}

		std::vector<Fence> fences;
		fences.reserve(count);
		for (size_t i = 0; i < count; i++)
			fences.emplace_back(flags);

		const int id = m_fence_id++;

		auto [it1, ok1] = mFences.emplace(id, std::move(fences));
		if (!ok1) {
			std::cerr << std::format("[ERROR] SyncManager: Emplace fences for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mFenceIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] SyncManager: Fence '{}' has not been recorded!\n", name);
			mFences.erase(id); 
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<Fence>(vec.data(), vec.size()) }; 
	}

	std::pair<int, std::span<Fence>> SyncManager::GetFences(std::string name)
	{
		if (auto it = mFenceIDs.find(name); it != mFenceIDs.end())
			return GetFences(it->second);
		std::cerr << std::format("[ERROR] SyncManager: Fences with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<Fence>> SyncManager::GetFences(int id)
	{
		if (auto it = mFences.find(id); it != mFences.end())
			return { id, std::span<Fence>(it->second.data(), it->second.size())};
		std::cerr << std::format("[ERROR] SyncManager: Fences with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}


	bool SyncManager::HasFence(std::string name)
	{
		if (auto it = mFenceIDs.find(name); it != mFenceIDs.end())
			return HasFence(it->second);
		return false;
	}

	bool SyncManager::HasFence(int id)
	{
		return mFences.contains(id);
	}

	size_t SyncManager::GetFencesCount() const
	{
		return mFences.size();
	}

	std::pair<int, std::span<Semaphore>> SyncManager::CreateSemaphores(std::string name, uint32_t count, VkSemaphoreCreateFlags flags)
	{
		if (auto it = mSemaphoreIDs.find(name); it != mSemaphoreIDs.end())
			return RecreateSemaphores(it->second, count, flags);

		if (count == 0) {
			std::cerr << std::format("[ERROR] SyncManager: '{}' requested 0 Semaphores\n", name);
			return { M_INVALID_ID, {} };
		}

		std::vector<Semaphore> semaphores;
		semaphores.reserve(count);
		for (size_t i = 0; i < count; i++)
			semaphores.emplace_back(flags);

		const int id = m_semaphore_id++;

		auto [it1, ok1] = mSemaphores.emplace(id, std::move(semaphores));
		if (!ok1) {
			std::cerr << std::format("[ERROR] SyncManager: emplace Semaphores for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mSemaphoreIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] Semaphore '{}' has not been recorded!\n", name);
			mSemaphores.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<Semaphore>(vec.data(), vec.size()) };
	}

	std::pair<int, std::span<Semaphore>> SyncManager::GetSemaphores(std::string name)
	{
		if (auto it = mSemaphoreIDs.find(name); it != mSemaphoreIDs.end())
			return GetSemaphores(it->second);
		std::cerr << std::format("[ERROR] SyncManager: Semaphores with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<Semaphore>> SyncManager::GetSemaphores(int id)
	{
		if (auto it = mSemaphores.find(id); it != mSemaphores.end())
			return { id, std::span<Semaphore>(it->second.data(), it->second.size()) };
		std::cerr << std::format("[ERROR] SyncManager: Semaphores with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	bool SyncManager::HasSemaphore(std::string name)
	{
		if (auto it = mSemaphoreIDs.find(name); it != mSemaphoreIDs.end())
			return HasSemaphore(it->second);
		return false;
	}

	bool SyncManager::HasSemaphore(int id)
	{
		return mSemaphores.contains(id);
	}

	size_t SyncManager::GetSemaphoresCount() const
	{
		return mSemaphores.size();
	}

#pragma endregion

}