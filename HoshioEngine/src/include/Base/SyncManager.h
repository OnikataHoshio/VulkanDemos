#ifndef _SYNC_MANAGER_H_
#define _SYNC_MANAGER_H_

#include "Base/VulkanBase.h"

namespace HoshioEngine {
	class Fence {
	private:
		VkFence handle = VK_NULL_HANDLE;
	public:
		Fence(VkFenceCreateFlags flags = 0);
		Fence(VkFenceCreateInfo& createInfo);
		Fence(Fence&& other) noexcept;
		~Fence();
		operator VkFence() const;
		const VkFence* Address() const;

		void Wait() const;
		void Reset() const;
		void WaitAndReset() const;
		void Status() const;
		void Create(VkFenceCreateInfo& createInfo);
		void Create(VkFenceCreateFlags flags = 0);
	};

	class Semaphore {
	private:
		VkSemaphore handle = VK_NULL_HANDLE;
	public:
		Semaphore(VkSemaphoreCreateFlags flags = 0);
		Semaphore(VkSemaphoreCreateInfo& createInfo);
		Semaphore(Semaphore&& other) noexcept;
		~Semaphore();
		operator VkSemaphore() const;
		const VkSemaphore* Address() const;

		void Create(VkSemaphoreCreateInfo& createInfo);

		void Create(VkSemaphoreCreateFlags flags = 0);
	};

	class SyncManager {
	private:
		int m_fence_id = 0;
		int m_semaphore_id = 0;

		std::unordered_map<std::string, int> mFenceIDs;
		std::unordered_map<int, std::vector<Fence>> mFences;
		std::unordered_map<std::string, int> mSemaphoreIDs;
		std::unordered_map<int, std::vector<Semaphore>> mSemaphores;

		std::pair<int, std::span<Fence>> RecreateFences(int id, uint32_t count, VkFenceCreateFlags flags = 0);
		std::pair<int, std::span<Semaphore>> RecreateSemaphores(int id, uint32_t count, VkSemaphoreCreateFlags flags = 0);

	public:
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

	};

}

#endif