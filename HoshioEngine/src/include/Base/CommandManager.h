#ifndef _COMMAND_MANAGER_H_
#define _COMMAND_MANAGER_H_

#include "Base/VulkanBase.h"

namespace HoshioEngine {
	class CommandBuffer {
	private:
		friend class CommandPool;
		VkCommandBuffer handle = VK_NULL_HANDLE;
	public:
		CommandBuffer() = default;
		CommandBuffer(CommandBuffer&& other) noexcept;
		operator VkCommandBuffer() const;
		const VkCommandBuffer* Address() const;

		void Begin(VkCommandBufferUsageFlags usageFlags = 0) const;

		void End() const;
	};

	class CommandPool {
	private:
		VkCommandPool handle = VK_NULL_HANDLE;
	public:
		CommandPool() = default;
		CommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
		CommandPool(VkCommandPoolCreateInfo& createInfo);
		CommandPool(CommandPool&& other);
		~CommandPool();
		operator VkCommandPool() const;
		const VkCommandPool* Address() const;

		void Allocate(ArrayRef<VkCommandBuffer> buffers, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;

		void Allocate(ArrayRef<CommandBuffer> buffers, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;

		void Free(ArrayRef<VkCommandBuffer> buffers) const;

		void Free(ArrayRef<CommandBuffer> bufffers) const;

		void Create(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

		void Create(VkCommandPoolCreateInfo& createInfo);
	};




}

#endif // !_COMMAND_MANAGER_H_
