#include "Base/MemoryManager.h"

namespace HoshioEngine {

#pragma region DeviceMemory

	VkDeviceSize DeviceMemory::AdjustNoCoherentMemorySize(VkDeviceSize& size, VkDeviceSize& offset) const
	{
		const VkDeviceSize& atomSize = VulkanBase::Base().PhysicalDeviceProperties().limits.nonCoherentAtomSize;
		VkDeviceSize _offset = offset;
		VkDeviceSize memEnd = _offset + size;
		offset = offset / atomSize * atomSize;
		memEnd = (memEnd + atomSize - 1) / atomSize * atomSize;
		memEnd = std::min(memEnd, allocationSize);
		size = memEnd - offset;
		return _offset - offset;
	}

	DeviceMemory::DeviceMemory(VkMemoryAllocateInfo& allocationInfo)
	{
		Allocate(allocationInfo);
	}

	DeviceMemory::DeviceMemory(DeviceMemory&& other) noexcept
	{
		handle = other.handle;
		allocationSize = other.allocationSize;
		memoryPropertyFlags = other.memoryPropertyFlags;
		other.handle = VK_NULL_HANDLE;
		other.allocationSize = 0;
		other.memoryPropertyFlags = 0;
	}

	DeviceMemory::~DeviceMemory()
	{
		if (handle) {
			vkFreeMemory(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
		allocationSize = 0; 
		memoryPropertyFlags = 0;
	}

	DeviceMemory::operator VkDeviceMemory() const
	{
		return handle;
	}

	const VkDeviceMemory* DeviceMemory::Address() const
	{
		return &handle;
	}

	VkDeviceSize DeviceMemory::AllocationSize() const
	{
		return allocationSize;
	}

	VkMemoryPropertyFlags DeviceMemory::MemoryPropertyFlags() const
	{
		return memoryPropertyFlags;
	}

	void DeviceMemory::MapMemory(void*& pData, VkDeviceSize size, VkDeviceSize offset) const
	{
		VkDeviceSize deltaOffset = 0;
		if (!(memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
			deltaOffset = AdjustNoCoherentMemorySize(size, offset);
		if (vkMapMemory(VulkanBase::Base().Device(), handle, offset, size, 0, &pData) != VK_SUCCESS)
			throw std::runtime_error("Failed to map the memory");
		if (!(memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
			pData = static_cast<uint8_t*>(pData) + deltaOffset;
			VkMappedMemoryRange range = {
				.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
				.memory = handle,
				.offset = offset,
				.size = size
			};
			if (vkInvalidateMappedMemoryRanges(VulkanBase::Base().Device(), 1, &range) != VK_SUCCESS)
				throw std::runtime_error("Failed to invalidate mapped memory range!");
		}
	}

	void DeviceMemory::UnMapMemory(VkDeviceSize size, VkDeviceSize offset) const
	{
		if (!(memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
			AdjustNoCoherentMemorySize(size, offset);
			VkMappedMemoryRange range = {
				.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
				.memory = handle,
				.offset = offset,
				.size = size
			};
			if (vkFlushMappedMemoryRanges(VulkanBase::Base().Device(), 1, &range) != VK_SUCCESS)
				throw std::runtime_error("Failed to flush mapped memory range!");
		}
		vkUnmapMemory(VulkanBase::Base().Device(), handle);
	}

	void DeviceMemory::SynchronizeData(const void* pData_src, VkDeviceSize size, VkDeviceSize offset) const
	{
		void* pData_dst;
		MapMemory(pData_dst, size, offset);
		memcpy(pData_dst, pData_src, size_t(size));
		UnMapMemory(size, offset);
	}

	void DeviceMemory::RetrieveData(void* pData_dst, VkDeviceSize size, VkDeviceSize offset) const
	{
		void* pData_src;
		MapMemory(pData_src, size, offset);
		memcpy(pData_dst, pData_src, size_t(size));
		UnMapMemory(size, offset);
	}

	bool DeviceMemory::Allocate(VkMemoryAllocateInfo& allocationInfo)
	{
		if (allocationInfo.memoryTypeIndex >= VulkanBase::Base().PhysicalDeviceMemoryProperties().memoryTypeCount) {
			std::cout << std::format("[ DeviceMemory ]WARNING\nMemoryTypeIndex Index is larger than memoryTypeCount.\n");
			return false;
		}

		allocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

		if (vkAllocateMemory(VulkanBase::Base().Device(), &allocationInfo, nullptr, &handle) != VK_SUCCESS) {
			std::cout << std::format("[ DeviceMemory ]WARNING\nFailed to allocate memory.\n");
			return false;
		}

		allocationSize = allocationInfo.allocationSize;

		memoryPropertyFlags = VulkanBase::Base().PhysicalDeviceMemoryProperties().memoryTypes[allocationInfo.memoryTypeIndex].propertyFlags;

		return true;
	}

#pragma endregion

#pragma region Buffer

	Buffer::Buffer(VkBufferCreateInfo& createInfo)
	{
		Create(createInfo);
	}

	Buffer::Buffer(Buffer&& other) noexcept
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}

	Buffer::~Buffer()
	{
		if (handle) {
			vkDestroyBuffer(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}

	Buffer::operator VkBuffer() const
	{
		return handle;
	}

	const VkBuffer* Buffer::Address() const
	{
		return &handle;
	}

	VkMemoryAllocateInfo Buffer::MemoryAllocateInfo(VkMemoryPropertyFlags desiredMemoryProperties) const
	{
		VkMemoryAllocateInfo allocateInfo = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
		};
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(VulkanBase::Base().Device(), handle, &memoryRequirements);
		
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = UINT32_MAX;

		const auto& memoryProperties = VulkanBase::Base().PhysicalDeviceMemoryProperties();

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
			if ((memoryRequirements.memoryTypeBits & 1 << i) &&
				(memoryProperties.memoryTypes[i].propertyFlags & desiredMemoryProperties) == desiredMemoryProperties) {
				allocateInfo.memoryTypeIndex = i;
				break;
			}
		}

		return allocateInfo;
	}

	void Buffer::BindMemory(VkDeviceMemory deviceMemory, VkDeviceSize memoryOffset) const
	{
		if (vkBindBufferMemory(VulkanBase::Base().Device(), handle, deviceMemory, memoryOffset) != VK_SUCCESS)
			throw std::runtime_error("Failed to bind buffer and memory.");
	}

	void Buffer::Create(VkBufferCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		if (vkCreateBuffer(VulkanBase::Base().Device(), &createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a buffer.");
	}

#pragma endregion

#pragma region BufferMemory

	BufferMemory::BufferMemory(VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags desiredMemoryProperties)
	{
		Create(createInfo, desiredMemoryProperties);
	}

	BufferMemory::BufferMemory(BufferMemory&& other) noexcept : 
		buffer(std::move(other.buffer)), deviceMemory(std::move(other.deviceMemory))
	{
		areBound = other.areBound;
	}

	BufferMemory::~BufferMemory()
	{
		areBound = false;
	}

	VkDeviceMemory BufferMemory::DeviceMemory() const
	{
		return deviceMemory;
	}

	const VkDeviceMemory* BufferMemory::AddressOfDeviceMemory() const
	{
		return deviceMemory.Address();
	}

	VkBuffer BufferMemory::Buffer() const
	{
		return buffer;
	}

	const VkBuffer* BufferMemory::AddressOfBuffer() const
	{
		return buffer.Address();
	}

	bool BufferMemory::AreBound() const
	{
		return areBound;
	}

	VkDeviceSize BufferMemory::AllocationSize() const
	{
		return deviceMemory.AllocationSize();
	}

	VkMemoryPropertyFlags BufferMemory::MemoryPropertyFlags() const
	{
		return deviceMemory.MemoryPropertyFlags();
	}

	void BufferMemory::MapMemory(void*& pData, VkDeviceSize size, VkDeviceSize offset) const
	{
		deviceMemory.MapMemory(pData, size, offset);
	}

	void BufferMemory::UnMapMemory(VkDeviceSize size, VkDeviceSize offset) const
	{
		deviceMemory.UnMapMemory(size, offset);
	}

	void BufferMemory::SynchronizeData(const void* pData_src, VkDeviceSize size, VkDeviceSize offset) const
	{
		deviceMemory.SynchronizeData(pData_src, size, offset);
	}

	void BufferMemory::RetrieveData(void* pData_dst, VkDeviceSize size, VkDeviceSize offset) const
	{
		deviceMemory.RetrieveData(pData_dst, size, offset);
	}

	void BufferMemory::CreateBuffer(VkBufferCreateInfo& createInfo)
	{
		buffer.Create(createInfo);
	}

	bool BufferMemory::AllocateMemory(VkMemoryPropertyFlags desiredMemoryProperties)
	{
		VkMemoryAllocateInfo allocateInfo = buffer.MemoryAllocateInfo(desiredMemoryProperties);
		if (allocateInfo.memoryTypeIndex >= VulkanBase::Base().PhysicalDeviceMemoryProperties().memoryTypeCount) {
			std::cout << std::format("[ BufferMemory ] WARNING\nMemory Type Index out of range of memory type count.\n");
			return false;
		}
		return deviceMemory.Allocate(allocateInfo);
	}

	void BufferMemory::BindMemory()
	{
		buffer.BindMemory(deviceMemory);
		areBound = true;
	}

	void BufferMemory::Create(VkBufferCreateInfo& createInfo, VkMemoryPropertyFlags desiredMemoryProperties)
	{
		CreateBuffer(createInfo);
		AllocateMemory(desiredMemoryProperties);
		BindMemory();
	}

#pragma endregion

#pragma region BufferView

	BufferView::BufferView(VkBufferViewCreateInfo& createInfo)
	{
		Create(createInfo);
	}

	BufferView::BufferView(VkBuffer buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range)
	{
		Create(buffer, format, offset, range);
	}

	BufferView::BufferView(BufferView&& other) noexcept
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}

	BufferView::~BufferView()
	{
		if (handle) {
			vkDestroyBufferView(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}

	BufferView::operator VkBufferView() const
	{
		return handle;
	}

	const VkBufferView* BufferView::Address() const
	{
		return &handle;
	}

	void BufferView::Create(VkBuffer buffer, VkFormat format, VkDeviceSize offset, VkDeviceSize range)
	{
		VkBufferViewCreateInfo createInfo = {
			.buffer = buffer,
			.format = format,
			.offset = offset,
			.range = range
		};
		Create(createInfo);
	}

	void BufferView::Create(VkBufferViewCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
		if (vkCreateBufferView(VulkanBase::Base().Device(), &createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a buffer view.");
	}

#pragma endregion

#pragma region Image

	Image::Image(VkImageCreateInfo& createInfo)
	{
		Create(createInfo);
	}

	Image::Image(Image&& other) noexcept
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}

	Image::~Image()
	{
		if (handle) {
			vkDestroyImage(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}

	Image::operator VkImage() const
	{
		return handle;
	}

	const VkImage* Image::Address() const
	{
		return &handle;
	}

	VkMemoryAllocateInfo Image::MemoryAllocateInfo(VkMemoryPropertyFlags desiredMemoryProperties) const
	{
		VkMemoryAllocateInfo allocateInfo = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
		};
		VkMemoryRequirements requirements;
		vkGetImageMemoryRequirements(VulkanBase::Base().Device(), handle, &requirements);
		allocateInfo.allocationSize = requirements.size;
		allocateInfo.memoryTypeIndex = UINT32_MAX;
		auto GetMemoryTypeIndex = [](uint32_t memoryTypeBits, VkMemoryPropertyFlags desiredMemoryProperties) {
			const auto& memoryProperties = VulkanBase::Base().PhysicalDeviceMemoryProperties();
			for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
				if ((memoryTypeBits & 1 << i) &&
					(desiredMemoryProperties & memoryProperties.memoryTypes[i].propertyFlags) == desiredMemoryProperties)
					return i;
			}
			return UINT32_MAX;
		};
		allocateInfo.memoryTypeIndex = GetMemoryTypeIndex(requirements.memoryTypeBits, desiredMemoryProperties);
		if (allocateInfo.memoryTypeIndex == UINT32_MAX)
			allocateInfo.memoryTypeIndex = GetMemoryTypeIndex(requirements.memoryTypeBits,
				desiredMemoryProperties & (~VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT));
		return allocateInfo;
	}

	void Image::BindMemory(VkDeviceMemory deviceMemory, VkDeviceSize memoryOffset) const
	{
		if (vkBindImageMemory(VulkanBase::Base().Device(), handle, deviceMemory, memoryOffset) != VK_SUCCESS)
			throw std::runtime_error("Failed to bind image to device memory.");
	}

	void Image::Create(VkImageCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		if (vkCreateImage(VulkanBase::Base().Device(), &createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create image");
	}

#pragma endregion

#pragma region ImageMemory

	ImageMemory::ImageMemory(VkImageCreateInfo& createInfo, VkMemoryPropertyFlags desiredMemoryProperties)
	{
		Create(createInfo, desiredMemoryProperties);
	}

	ImageMemory::ImageMemory(ImageMemory&& other) noexcept :
		deviceMemory(std::move(other.deviceMemory)), image(std::move(other.image))
	{
		areBound = false;
	}

	ImageMemory::~ImageMemory()
	{
		areBound = false;
	}

	VkDeviceMemory ImageMemory::DeviceMemory() const
	{
		return deviceMemory;
	}

	const VkDeviceMemory* ImageMemory::AddressOfDeviceMemory() const
	{
		return deviceMemory.Address();
	}

	VkImage ImageMemory::Image() const
	{
		return image;
	}

	const VkImage* ImageMemory::AddressOfImage() const
	{
		return image.Address();
	}

	bool ImageMemory::AreBound() const
	{
		return areBound;
	}

	VkDeviceSize ImageMemory::AllocationSize() const
	{
		return deviceMemory.AllocationSize();
	}

	VkMemoryPropertyFlags ImageMemory::MemoryPropertyFlags() const
	{
		return deviceMemory.MemoryPropertyFlags();
	}

	void ImageMemory::MapMemory(void*& pData, VkDeviceSize size, VkDeviceSize offset) const
	{
		return deviceMemory.MapMemory(pData, size, offset);
	}

	void ImageMemory::UnMapMemory(VkDeviceSize size, VkDeviceSize offset) const
	{
		return deviceMemory.UnMapMemory(size, offset);
	}

	void ImageMemory::SynchronizeData(const void* pData_src, VkDeviceSize size, VkDeviceSize offset) const
	{
		return deviceMemory.SynchronizeData(pData_src, size, offset);
	}

	void ImageMemory::RetrieveData(void* pData_dst, VkDeviceSize size, VkDeviceSize offset) const
	{
		return deviceMemory.RetrieveData(pData_dst, size, offset);
	}

	void ImageMemory::CreateImage(VkImageCreateInfo& createInfo)
	{
		image.Create(createInfo);
	}

	bool ImageMemory::AllocateMemory(VkMemoryPropertyFlags desiredMemoryProperties)
	{
		VkMemoryAllocateInfo allocateInfo = image.MemoryAllocateInfo(desiredMemoryProperties);
		if (allocateInfo.memoryTypeIndex >= VulkanBase::Base().PhysicalDeviceMemoryProperties().memoryTypeCount) {
			std::cout << std::format("[ ImageMemory ]\nMemory type index out of range of memory type count!\n");
			return false;
		}
		return deviceMemory.Allocate(allocateInfo);
	}

	void ImageMemory::BindMemory()
	{
		image.BindMemory(deviceMemory);
	}

	void ImageMemory::Create(VkImageCreateInfo& createInfo, VkMemoryPropertyFlags desiredMemoryProperties)
	{
		CreateImage(createInfo);
		AllocateMemory(desiredMemoryProperties);
		BindMemory();
	}

#pragma endregion

#pragma region ImageView

	ImageView::ImageView(VkImageViewCreateInfo& createInfo)
	{
		Create(createInfo);
	}

	ImageView::ImageView(VkImage image, VkImageViewType viewType, VkFormat format, const VkImageSubresourceRange& subresourceRange, const VkComponentMapping& components, VkImageViewCreateFlags flags)
	{
		Create(image, viewType, format, subresourceRange, components, flags);
	}

	ImageView::ImageView(ImageView&& other) noexcept
	{
		handle = other.handle;
		other.handle = VK_NULL_HANDLE;
	}

	ImageView::~ImageView()
	{
		if (handle) {
			vkDestroyImageView(VulkanBase::Base().Device(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}

	ImageView::operator VkImageView() const
	{
		return handle;
	}

	const VkImageView* ImageView::Address() const
	{
		return &handle;
	}

	void ImageView::Create(VkImageViewCreateInfo& createInfo)
	{
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		if (vkCreateImageView(VulkanBase::Base().Device(), &createInfo, nullptr, &handle) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a image view");
	}

	void ImageView::Create(VkImage image, VkImageViewType viewType, VkFormat format, const VkImageSubresourceRange& subresourceRange, const VkComponentMapping& components, VkImageViewCreateFlags flags)
	{
		VkImageViewCreateInfo createInfo = {
			.flags = flags,
			.image = image,
			.viewType = viewType,
			.format = format,
			.components = components,
			.subresourceRange = subresourceRange,
		};
		Create(createInfo);
	}

#pragma endregion

}