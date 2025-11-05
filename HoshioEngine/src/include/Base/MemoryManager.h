#ifndef _MEMORY_MANAGER_H_
#define _MEMORY_MANAGER_H_

#include "Base/VulkanBase.h"

namespace HoshioEngine {
	class DeviceMemory {
	private:
		VkDeviceMemory handle = VK_NULL_HANDLE;
		VkDeviceSize allocationSize = 0;
		VkMemoryPropertyFlags memoryPropertyFlags = 0;

		VkDeviceSize AdjustNoCoherentMemorySize(VkDeviceSize& size, VkDeviceSize& offset) const;

	public:
		DeviceMemory() = default;
		DeviceMemory(VkMemoryAllocateInfo& allocationInfo);
		DeviceMemory(DeviceMemory&& other) noexcept;
		~DeviceMemory();
		operator VkDeviceMemory() const;
		const VkDeviceMemory* Address() const;

		VkDeviceSize AllocationSize() const;
		VkMemoryPropertyFlags MemoryPropertyFlags() const;

		void MapMemory(void*& pData, VkDeviceSize size, VkDeviceSize offset) const;

		void UnMapMemory(VkDeviceSize size, VkDeviceSize offset = 0) const;

		void SynchronizeData(const void* pData_src, VkDeviceSize size, VkDeviceSize offset = 0) const;

		void RetrieveData(void* pData_dst, VkDeviceSize size, VkDeviceSize offset = 0) const;

		bool Allocate(VkMemoryAllocateInfo& allocationInfo);
	};

	class Buffer {
	private:
		VkBuffer handle = VK_NULL_HANDLE;
	public:
		Buffer() = default;
		Buffer(VkBufferCreateInfo& createInfo);
		Buffer(Buffer&& other) noexcept;
		~Buffer();
		operator VkBuffer() const;
		const VkBuffer* Address() const;

		VkMemoryAllocateInfo MemoryAllocateInfo(VkMemoryPropertyFlags desiredMemoryProperties) const;

		void BindMemory(VkDeviceMemory deviceMemory, VkDeviceSize memoryOffset = 0) const;

		void Create(VkBufferCreateInfo& createInfo);
	};

	class BufferMemory {
	private:
		DeviceMemory deviceMemory;
		Buffer buffer;
		bool areBound = false;
	public:
		BufferMemory() = default;
		BufferMemory(VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags desiredMemoryProperties);
		BufferMemory(BufferMemory&& other) noexcept;
		~BufferMemory();
		
		VkDeviceMemory DeviceMemory() const;
		const VkDeviceMemory* AddressOfDeviceMemory() const;
		VkBuffer Buffer() const;
		const VkBuffer* AddressOfBuffer() const;
		bool AreBound() const;

		VkDeviceSize AllocationSize() const;
		VkMemoryPropertyFlags MemoryPropertyFlags() const;

		void MapMemory(void*& pData, VkDeviceSize size, VkDeviceSize offset = 0) const;
		void UnMapMemory(VkDeviceSize size, VkDeviceSize offset = 0) const;
		void SynchronizeData(const void* pData_src, VkDeviceSize size, VkDeviceSize offset = 0) const;
		void RetrieveData(void* pData_dst, VkDeviceSize size, VkDeviceSize offset = 0) const;

		void CreateBuffer(VkBufferCreateInfo& createInfo);
		bool AllocateMemory(VkMemoryPropertyFlags desiredMemoryProperties);
		void BindMemory();

		void Create(VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags desiredMemoryProperties);
	};

	class BufferView {
	private:
		VkBufferView handle = VK_NULL_HANDLE;
	public:
		BufferView() = default;
		BufferView(VkBufferViewCreateInfo& createInfo);
		BufferView(VkBuffer buffer, VkFormat format, VkDeviceSize offset = 0, VkDeviceSize range = 0);
		BufferView(BufferView&& other) noexcept;
		~BufferView();
		operator VkBufferView() const;
		const VkBufferView* Address() const;

		void Create(VkBuffer buffer, VkFormat format, VkDeviceSize offset = 0, VkDeviceSize range = 0);
		void Create(VkBufferViewCreateInfo& createInfo);

	};

	class Image {
	private:
		VkImage handle = VK_NULL_HANDLE;
	public:
		Image() = default;
		Image(VkImageCreateInfo& createInfo);
		Image(Image&& other) noexcept;
		~Image();
		operator VkImage() const;
		const VkImage* Address() const;

		VkMemoryAllocateInfo MemoryAllocateInfo(VkMemoryPropertyFlags desiredMemoryProperties) const;

		void BindMemory(VkDeviceMemory deviceMemory, VkDeviceSize memoryOffset = 0) const;

		void Create(VkImageCreateInfo& createInfo);
		
	};

	class ImageMemory {
	private:
		DeviceMemory deviceMemory;
		Image image;
		bool areBound = false;
	public:
		ImageMemory() = default;
		ImageMemory(VkImageCreateInfo& createInfo, VkMemoryPropertyFlags desiredMemoryProperties);
		ImageMemory(ImageMemory&& other) noexcept;
		~ImageMemory();
		
		VkDeviceMemory DeviceMemory() const;
		const VkDeviceMemory* AddressOfDeviceMemory() const;
		VkImage Image() const;
		const VkImage* AddressOfImage() const;
		bool AreBound() const;

		VkDeviceSize AllocationSize() const;
		VkMemoryPropertyFlags MemoryPropertyFlags() const;

		void MapMemory(void*& pData, VkDeviceSize size, VkDeviceSize offset = 0) const;
		void UnMapMemory(VkDeviceSize size, VkDeviceSize offset = 0) const;
		void SynchronizeData(const void* pData_src, VkDeviceSize size, VkDeviceSize offset = 0) const;
		void RetrieveData(void* pData_dst, VkDeviceSize size, VkDeviceSize offset = 0) const;

		void CreateImage(VkImageCreateInfo& createInfo);
		bool AllocateMemory(VkMemoryPropertyFlags desiredMemoryProperties);
		void BindMemory();

		void Create(VkImageCreateInfo& createInfo, VkMemoryPropertyFlags desiredMemoryProperties);
	};

	class ImageView {
	private:
		VkImageView handle = VK_NULL_HANDLE;
	public:
		ImageView() = default;
		ImageView(VkImageViewCreateInfo& createInfo);
		ImageView(VkImage image, VkImageViewType viewType, VkFormat format,
			const VkImageSubresourceRange& subresourceRange, const VkComponentMapping& components, VkImageViewCreateFlags flags = 0);
		ImageView(ImageView&& other) noexcept;
		~ImageView();

		operator VkImageView() const;
		const VkImageView* Address() const;

		void Create(VkImageViewCreateInfo& createInfo);
		void Create(VkImage image, VkImageViewType viewType, VkFormat format,
			const VkImageSubresourceRange& subresourceRange, const VkComponentMapping& components,VkImageViewCreateFlags flags = 0);
	};

}

#endif // !_MEMORY_MANAGER_H
