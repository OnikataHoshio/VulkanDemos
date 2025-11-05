#ifndef _BUFFER_MANAGER_H_
#define _BUFFER_MANAGER_H_

#include "Base/MemoryManager.h"

namespace HoshioEngine {
	class StagingBuffer {
	protected:
		BufferMemory bufferMemory;
		VkDeviceSize memorySize = 0;
		Image alisedImage;
	public:
		StagingBuffer() = default;
		StagingBuffer(VkDeviceSize size);

		operator VkBuffer() const;
		const VkBuffer* Address() const;
		VkDeviceSize AllocationSize() const;

		void RetrieveData(void* pData_src, VkDeviceSize size) const;

		void SynchronizeData(const void* pData_src, VkDeviceSize size);

		void Expand(VkDeviceSize size);

		void Release();

		void* MapMemory(VkDeviceSize size);

		void UnMapMemory();

		[[nodiscard]]
		VkImage AliasedImage2D(VkFormat format, VkExtent2D extent);
	};

	class StagingBuffer_MainThread {
	private:
		StagingBuffer_MainThread() = default;
		~StagingBuffer_MainThread() = default;
		StagingBuffer_MainThread(StagingBuffer_MainThread&& other) = delete;
		StagingBuffer_MainThread(const StagingBuffer_MainThread& other) = delete;
		StagingBuffer_MainThread& operator=(const StagingBuffer_MainThread& single) = delete;
	public:
		static StagingBuffer& Main();

		static void RetrieveData(void* pData_src, VkDeviceSize size);

		static void SynchronizeData(const void* pData_src, VkDeviceSize size);

		static void Expand(VkDeviceSize size);

		static void Release();

		static void* MapMemory(VkDeviceSize size);

		static void UnMapMemory();

		static VkImage AliasedImage2D(VkFormat format, VkExtent2D extent);
	};

	class DeviceLocalBuffer {
	protected:
		BufferMemory bufferMemory;
	public:
		DeviceLocalBuffer() = default;
		DeviceLocalBuffer(VkDeviceSize size, VkBufferUsageFlags desiredUsages_without_transfer_dst);
		operator VkBuffer() const;
		const VkBuffer* Address() const;
		VkDeviceSize AllocationSize() const;

		void CmdUpdateBuffer(VkCommandBuffer commandBuffer, const void* pData_src, VkDeviceSize size_limited_to_65536, VkDeviceSize offset = 0) const;

		void CmdUpdateBuffer(VkCommandBuffer commandBuffer, const auto& data_src) const {
			vkCmdUpdateBuffer(commandBuffer, bufferMemory.Buffer(), 0, sizeof data_src, &data_src);
		}

		void CmdCopyBuffer(const void* pData_src, VkDeviceSize size, VkDeviceSize offset = 0) const;

		void TransferData(const void* pData_src, VkDeviceSize size, VkDeviceSize offset = 0) const;

		void TransferData(const auto& data_src) const {
			TransferData(&data_src, sizeof data_src);
		};

		void Create(VkDeviceSize size, VkBufferUsageFlags desiredUsages_without_transfer_dst);

		void Recreate(VkDeviceSize size, VkBufferUsageFlags desiredUsages_without_transfer_dst);
	};

	class VertexBuffer : public DeviceLocalBuffer {
	public:
		VertexBuffer() = default;
		VertexBuffer(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0);
		VertexBuffer& Create(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0);
		VertexBuffer& Recreate(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0);
	};

	class IndexBuffer : public DeviceLocalBuffer {
	public:
		IndexBuffer() = default;
		IndexBuffer(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0);
		IndexBuffer& Create(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0);
		IndexBuffer& Recreate(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0);
	};

	class UniformBuffer : public DeviceLocalBuffer {
	public:
		UniformBuffer() = default;
		UniformBuffer(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0);
		UniformBuffer& Create(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0);
		UniformBuffer& Recreate(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0);

		static VkDeviceSize CalculateAlignedSize(VkDeviceSize dataSize);
	};

	class StorageBuffer : public DeviceLocalBuffer {
	public:
		StorageBuffer() = default;
		StorageBuffer(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0);
		StorageBuffer& Create(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0);
		StorageBuffer& Recreate(VkDeviceSize size, VkBufferUsageFlags otherUsages = 0);

		static VkDeviceSize CalculateAlignedSize(VkDeviceSize dataSize);
	};
}

#endif // !_BUFFER_MANAGER_H_
