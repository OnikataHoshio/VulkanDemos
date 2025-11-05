#include "Base/CommandManager.h"

namespace HoshioEngine {

#pragma region CommandBuffer

	CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}

	CommandBuffer::operator VkCommandBuffer() const
	{
		return handle;
	}
	const VkCommandBuffer* CommandBuffer::Address() const
	{
		return &handle;
	}
	void CommandBuffer::Begin(VkCommandBufferUsageFlags usageFlags) const
	{
		VkCommandBufferBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = usageFlags
		};
		if (vkBeginCommandBuffer(handle, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to start a commandBuffer");
	}
	void CommandBuffer::End() const
	{
		if (vkEndCommandBuffer(handle))
			throw std::runtime_error("Failed to end a commandBuffer");
	}

#pragma endregion


#pragma region CommandPool

	CommandPool::CommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
	{
		Create(queueFamilyIndex, flags);
	}
	CommandPool::CommandPool(VkCommandPoolCreateInfo& createInfo)
	{
		Create(createInfo);
	}
	CommandPool::CommandPool(CommandPool&& other)
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}
	CommandPool::~CommandPool()
	{
		if (handle) {
			vkDestroyCommandPool(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}
	CommandPool::operator VkCommandPool() const
	{
		return handle;
	}
	const VkCommandPool* CommandPool::Address() const
	{
		return &handle;
	}
	void CommandPool::Allocate(ArrayRef<VkCommandBuffer> buffers, VkCommandBufferLevel level) const
	{
		VkCommandBufferAllocateInfo allocateInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = handle,
			.level = level,
			.commandBufferCount = static_cast<uint32_t>(buffers.size())
		};
		if (vkAllocateCommandBuffers(VulkanBase::Base().Device(), &allocateInfo, buffers.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffers");
	}

	void CommandPool::Allocate(ArrayRef<CommandBuffer> buffers, VkCommandBufferLevel level) const
	{
		Allocate({ &buffers[0].handle, buffers.size() }, level);
	}

	void CommandPool::Free(ArrayRef<VkCommandBuffer> buffers) const
	{
		vkFreeCommandBuffers(VulkanBase::Base().Device(), handle, buffers.size(), buffers.data());
		memset(buffers.begin(), 0, buffers.size() * sizeof(VkCommandBuffer));
	}

	void CommandPool::Free(ArrayRef<CommandBuffer> buffers) const
	{
		Free({&buffers[0].handle, buffers.size()});
	}

	void CommandPool::Create(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
	{
		VkCommandPoolCreateInfo createInfo = {
			.flags = flags,
			.queueFamilyIndex = queueFamilyIndex
		};
		Create(createInfo);
	}
	void CommandPool::Create(VkCommandPoolCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		if (vkCreateCommandPool(VulkanBase::Base().Device(), &createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a command pool");
	}

#pragma endregion

}