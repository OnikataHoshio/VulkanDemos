#include "Plus/BufferManager.h"
#include "Plus/VulkanPlus.h"

namespace HoshioEngine {

#pragma region StagingBuffer

	StagingBuffer::StagingBuffer(VkDeviceSize size)
	{
		Expand(size);
	}
	StagingBuffer::operator VkBuffer() const
	{
		return bufferMemory.Buffer();
	}
	const VkBuffer* StagingBuffer::Address() const
	{
		return bufferMemory.AddressOfBuffer();
	}
	VkDeviceSize StagingBuffer::AllocationSize() const
	{
		return bufferMemory.AllocationSize();
	}
	void StagingBuffer::RetrieveData(void* pData_src, VkDeviceSize size) const
	{
		bufferMemory.RetrieveData(pData_src, size);
	}
	void StagingBuffer::SynchronizeData(const void* pData_src, VkDeviceSize size)
	{
		Expand(size);
		bufferMemory.SynchronizeData(pData_src, size);
	}
	void StagingBuffer::Expand(VkDeviceSize size)
	{
		if (size <= AllocationSize())
			return;
		Release();
		VkBufferCreateInfo createInfo = {
			.size = size,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
		};
		bufferMemory.Create(createInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	}
	void StagingBuffer::Release()
	{
		bufferMemory.~BufferMemory();
	}
	void* StagingBuffer::MapMemory(VkDeviceSize size)
	{
		Expand(size);
		void* pData_dst = nullptr;
		bufferMemory.MapMemory(pData_dst, size);
		memorySize = size;
		return pData_dst;
	}
	void StagingBuffer::UnMapMemory()
	{
		bufferMemory.UnMapMemory(memorySize);
		memorySize = 0;
	}
	VkImage StagingBuffer::AliasedImage2D(VkFormat format, VkExtent2D extent)
	{
		if (!(VulkanPlus::Plus().FormatProperties(format).linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT))
			return VK_NULL_HANDLE;

		VkDeviceSize imageDataSize = VkDeviceSize(vkuFormatElementSize(format)) * extent.width * extent.height;

		if (imageDataSize > AllocationSize())
			return VK_NULL_HANDLE;

		VkImageFormatProperties imageFormatProperties;
		vkGetPhysicalDeviceImageFormatProperties(VulkanBase::Base().PhysicalDevice(),
			format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 0, &imageFormatProperties);

		if (extent.width > imageFormatProperties.maxExtent.width ||
			extent.height > imageFormatProperties.maxExtent.height ||
			imageDataSize > imageFormatProperties.maxResourceSize)
			return VK_NULL_HANDLE;

		VkImageCreateInfo createInfo = {
			.imageType =VK_IMAGE_TYPE_2D,
			.format = format,
			.extent = VkExtent3D{extent.width, extent.height, 1},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_LINEAR,
			.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED
		};

		alisedImage.~Image();
		alisedImage.Create(createInfo);

		VkImageSubresource imageSubresouce = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.arrayLayer = 0
		};
		VkSubresourceLayout subresourceLayout;
		vkGetImageSubresourceLayout(VulkanBase::Base().Device(), alisedImage, &imageSubresouce, &subresourceLayout);
		if (subresourceLayout.size != imageDataSize)
			return VK_NULL_HANDLE;
		alisedImage.BindMemory(bufferMemory.DeviceMemory());
		return alisedImage;
	}

#pragma endregion

#pragma region StagingBuffer_MainThread

	StagingBuffer& StagingBuffer_MainThread::Main()
	{
		static StagingBuffer stagingBuffer;
		return stagingBuffer;
	}

	void StagingBuffer_MainThread::RetrieveData(void* pData_src, VkDeviceSize size)
	{
		Main().RetrieveData(pData_src, size);
	}

	void StagingBuffer_MainThread::SynchronizeData(const void* pData_src, VkDeviceSize size)
	{
		Main().SynchronizeData(pData_src, size);
	}

	void StagingBuffer_MainThread::Expand(VkDeviceSize size)
	{
		Main().Expand(size);
	}

	void StagingBuffer_MainThread::Release()
	{
		Main().Release();
	}

	void* StagingBuffer_MainThread::MapMemory(VkDeviceSize size)
	{
		return Main().MapMemory(size);
	}

	void StagingBuffer_MainThread::UnMapMemory()
	{
		Main().UnMapMemory();
	}

	VkImage StagingBuffer_MainThread::AliasedImage2D(VkFormat format, VkExtent2D extent) {
		return Main().AliasedImage2D(format, extent);
	}

#pragma endregion

#pragma region DeviceLocalBuffer

	DeviceLocalBuffer::DeviceLocalBuffer(VkDeviceSize size, VkBufferUsageFlags desiredUsages_without_transfer_dst)
	{
		Create(size, desiredUsages_without_transfer_dst);
	}

	DeviceLocalBuffer::operator VkBuffer() const
	{
		return bufferMemory.Buffer();
	}

	const VkBuffer* DeviceLocalBuffer::Address() const
	{
		return bufferMemory.AddressOfBuffer();
	}

	VkDeviceSize DeviceLocalBuffer::AllocationSize() const
	{
		return bufferMemory.AllocationSize();
	}

	void DeviceLocalBuffer::CmdUpdateBuffer(VkCommandBuffer commandBuffer, const void* pData_src, VkDeviceSize size_limited_to_65536, VkDeviceSize offset) const
	{
		vkCmdUpdateBuffer(commandBuffer, bufferMemory.Buffer(), offset, size_limited_to_65536, pData_src);
	}



	void DeviceLocalBuffer::CmdCopyBuffer(const void* pData_src, VkDeviceSize size, VkDeviceSize offset) const
	{
		if (bufferMemory.MemoryPropertyFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			bufferMemory.SynchronizeData(pData_src, size, offset);
			return;
		}

		StagingBuffer_MainThread::SynchronizeData(pData_src, size);
		const CommandBuffer& commandBuffer = VulkanPlus::Plus().CommandBuffer_Transfer();
		commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VkBufferCopy copyRegion = {
			.srcOffset = 0,
			.dstOffset = offset,
			.size = size,
		};
		vkCmdCopyBuffer(commandBuffer, StagingBuffer_MainThread::Main(), bufferMemory.Buffer(), 1, &copyRegion);
		commandBuffer.End();
		VulkanPlus::Plus().ExecuteCommandBuffer_Graphics(commandBuffer);
	}

	void DeviceLocalBuffer::TransferData(const void* pData_src, VkDeviceSize size, VkDeviceSize offset) const
	{
		if (bufferMemory.MemoryPropertyFlags() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			bufferMemory.SynchronizeData(pData_src, size, offset);
			return;
		}
		StagingBuffer_MainThread::SynchronizeData(pData_src, size);
		auto& commandBuffer = VulkanPlus::Plus().CommandBuffer_Transfer();
		commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		VkBufferCopy region = { 0, offset, size };
		vkCmdCopyBuffer(commandBuffer,StagingBuffer_MainThread::Main(), bufferMemory.Buffer(), 1, &region);
		commandBuffer.End();
		VulkanPlus::Plus().ExecuteCommandBuffer_Graphics(commandBuffer);
	}

	void DeviceLocalBuffer::Create(VkDeviceSize size, VkBufferUsageFlags desiredUsages_without_transfer_dst)
	{
		VkBufferCreateInfo createInfo = {
			.size = size,
			.usage = desiredUsages_without_transfer_dst | VK_BUFFER_USAGE_TRANSFER_DST_BIT
		};
		bufferMemory.CreateBuffer(createInfo);
		if (!bufferMemory.AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
			bufferMemory.AllocateMemory(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		bufferMemory.BindMemory();
	}

	void DeviceLocalBuffer::Recreate(VkDeviceSize size, VkBufferUsageFlags desiredUsages_without_transfer_dst)
	{
		VulkanBase::Base().WaitIdle();
		bufferMemory.~BufferMemory();
		Create(size, desiredUsages_without_transfer_dst);
	}

#pragma endregion

#pragma region VertexBuffer

	VertexBuffer::VertexBuffer(VkDeviceSize size, VkBufferUsageFlags otherUsages) :DeviceLocalBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT|otherUsages){}

	VertexBuffer& VertexBuffer::Create(VkDeviceSize size, VkBufferUsageFlags otherUsages)
	{
		DeviceLocalBuffer::Create(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | otherUsages);
		return *this;
	}

	VertexBuffer& VertexBuffer::Recreate(VkDeviceSize size, VkBufferUsageFlags otherUsages)
	{
		DeviceLocalBuffer::Recreate(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | otherUsages);
		return *this;
	}

#pragma endregion

#pragma region IndexBuffer

	IndexBuffer::IndexBuffer(VkDeviceSize size, VkBufferUsageFlags otherUsages) : DeviceLocalBuffer(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT|otherUsages){}

	IndexBuffer& IndexBuffer::Create(VkDeviceSize size, VkBufferUsageFlags otherUsages)
	{
		DeviceLocalBuffer::Create(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | otherUsages);
		return *this;
	}

	IndexBuffer& IndexBuffer::Recreate(VkDeviceSize size, VkBufferUsageFlags otherUsages)
	{
		DeviceLocalBuffer::Recreate(size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | otherUsages);
		return *this;
	}

#pragma endregion


#pragma region UniformBuffer

	UniformBuffer::UniformBuffer(VkDeviceSize size, VkBufferUsageFlags otherUsages):DeviceLocalBuffer(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | otherUsages){}

	UniformBuffer& UniformBuffer::Create(VkDeviceSize size, VkBufferUsageFlags otherUsages)
	{
		DeviceLocalBuffer::Create(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | otherUsages);
		return *this;
	}

	UniformBuffer& UniformBuffer::Recreate(VkDeviceSize size, VkBufferUsageFlags otherUsages)
	{
		DeviceLocalBuffer::Recreate(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | otherUsages);
		return *this;
	}

	VkDeviceSize UniformBuffer::CalculateAlignedSize(VkDeviceSize dataSize)
	{
		const VkDeviceSize& alignment = VulkanBase::Base().PhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
		return dataSize + alignment - 1 & ~(alignment - 1);
	}

#pragma endregion

#pragma region StorageBuffer

	StorageBuffer::StorageBuffer(VkDeviceSize size, VkBufferUsageFlags otherUsages):DeviceLocalBuffer(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT|otherUsages){}

	StorageBuffer& StorageBuffer::Create(VkDeviceSize size, VkBufferUsageFlags otherUsages)
	{
		DeviceLocalBuffer::Create(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | otherUsages);
		return *this;
	}

	StorageBuffer& StorageBuffer::Recreate(VkDeviceSize size, VkBufferUsageFlags otherUsages)
	{
		DeviceLocalBuffer::Recreate(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | otherUsages);
		return *this;
	}

	VkDeviceSize StorageBuffer::CalculateAlignedSize(VkDeviceSize dataSize)
	{
		const VkDeviceSize& alignment = VulkanBase::Base().PhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;
		return dataSize + alignment - 1 & ~(alignment - 1);
	}

#pragma endregion

}