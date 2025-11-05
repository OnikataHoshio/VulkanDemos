#include "Plus/ImageManager.h"
#include "Plus/VulkanPlus.h"
#include "Utils/ImageUtils.h"

namespace HoshioEngine {

#pragma region Texture

	void Texture::CreateImageMemory(VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t mipLevelCount, uint32_t arrayLayerCount, VkImageCreateFlags flags)
	{
		VkImageCreateInfo createInfo = {
			.imageType = imageType,
			.format = format,
			.extent = extent,
			.mipLevels = mipLevelCount,
			.arrayLayers = arrayLayerCount,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		imageMemory.Create(createInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	void Texture::CreateImageView(VkImageViewType viewType, VkFormat format, uint32_t mipLevelCount, uint32_t arrayLayerCount, VkImageViewCreateFlags flags)
	{
		imageView.Create(imageMemory.Image(), viewType, format, 
			VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevelCount, 0, arrayLayerCount }, VkComponentMapping{}, flags);
	}

	/*
	std::unique_ptr<uint8_t[]> Texture::LoadFile_Internal(const auto* address, size_t fileSize, VkExtent2D& extent, VkFormat format)
	{
#ifndef NDEBUG
		if (!(vkuFormatIsStoredFloat(format) && vkuFormatIs32bit(format)) ||
			!(vkuFormatIsStoredInt(format) && (vkuFormatIs8bit(format) || vkuFormatIs16bit(format))))
			throw std::runtime_error("Required format is not available for source image data!");
#endif // !NDEBUG
		int& width = reinterpret_cast<int&>(extent.width);
		int& height = reinterpret_cast<int&>(extent.height);
		int channelCount = 0;
		void* pImageData = nullptr;

		uint32_t componentCount = vkuGetFormatInfo(format).component_count;
		if constexpr (std::same_as<decltype(address), const char*>) {
			if (vkuFormatIsStoredInt(format)) 
				if (vkuFormatIs8bit(format))
					pImageData = stbi_load(address, &width, &height, &channelCount, componentCount);
				else
					pImageData = stbi_load_16(address, &width, &height, &channelCount, componentCount);
			else
				pImageData = stbi_loadf(address, &width, &height, &channelCount, componentCount);

			if (!pImageData)
				throw std::runtime_error(std::format("[ Texture ] ERROR\nFailed to load the file: {}\n", address));
		}

		if constexpr (std::same_as<decltype(address), const uint8_t*>) {
			if (fileSize > UINT32_MAX)
				throw std::runtime_error(std::format("[ Texture ] ERROR\nnFailed to load image data from the given address! Data size must be less than 2G!\n"));
			if (vkuFormatIsStoredInt(format))
				if (vkuFormatIs8bit(format))
					pImageData = stbi_load_from_memory(address, fileSize, &width, &height, &channelCount, componentCount);
				else
					pImageData = stbi_load_16_from_memory(address, fileSize, &width, &height, &channelCount, componentCount);
			else
				pImageData = stbi_loadf_from_memory(address, fileSize, &width, &height, &channelCount, componentCount);
			if (!pImageData)
				throw std::runtime_error(std::format("[ Texture ] ERROR\nFailed to load image data from the given address!\n"));
		}

		return std::unique_ptr<uint8_t[]>(static_cast<uint8_t*>(pImageData));
	}
	*/

	std::unique_ptr<uint8_t[]> Texture::LoadFile_Internal(const char* address, size_t fileSize, VkExtent2D& extent, VkFormat format)
	{
#ifndef NDEBUG
		if (!(vkuFormatIsStoredFloat(format) && vkuFormatIs32bit(format)) &&
			!(vkuFormatIsStoredInt(format) && (vkuFormatIs8bit(format) || vkuFormatIs16bit(format))))
			throw std::runtime_error("Required format is not available for source image data!");
#endif // !NDEBUG
		int& width = reinterpret_cast<int&>(extent.width);
		int& height = reinterpret_cast<int&>(extent.height);
		int channelCount = 0;
		void* pImageData = nullptr;

		uint32_t componentCount = vkuGetFormatInfo(format).component_count;

		if (vkuFormatIsStoredInt(format))
			if (vkuFormatIs8bit(format)) 
				pImageData = stbi_load(address, &width, &height, &channelCount, componentCount);
			else 
				pImageData = stbi_load_16(address, &width, &height, &channelCount, componentCount);
		else
			pImageData = stbi_loadf(address, &width, &height, &channelCount, componentCount);

		if (!pImageData)
			throw std::runtime_error(std::format("[ Texture ] ERROR\nFailed to load the file: {}\n", address));

		return std::unique_ptr<uint8_t[]>(static_cast<uint8_t*>(pImageData));
	}

	std::unique_ptr<uint8_t[]> Texture::LoadFile_Internal(const uint8_t* address, size_t fileSize, VkExtent2D& extent, VkFormat format)
	{
#ifndef NDEBUG
		if (!(vkuFormatIsStoredFloat(format) && vkuFormatIs32bit(format)) &&
			!(vkuFormatIsStoredInt(format) && (vkuFormatIs8bit(format) || vkuFormatIs16bit(format))))
			throw std::runtime_error("Required format is not available for source image data!");
#endif // !NDEBUG
		int& width = reinterpret_cast<int&>(extent.width);
		int& height = reinterpret_cast<int&>(extent.height);
		int channelCount = 0;
		void* pImageData = nullptr;

		uint32_t componentCount = vkuGetFormatInfo(format).component_count;

		if (fileSize > UINT32_MAX)
			throw std::runtime_error(std::format("[ Texture ] ERROR\nnFailed to load image data from the given address! Data size must be less than 2G!\n"));
		if (vkuFormatIsStoredInt(format))
			if (vkuFormatIs8bit(format))
				pImageData = stbi_load_from_memory(address, static_cast<int>(fileSize), &width, &height, &channelCount, componentCount);
			else
				pImageData = stbi_load_16_from_memory(address, static_cast<int>(fileSize), &width, &height, &channelCount, componentCount);
		else
			pImageData = stbi_loadf_from_memory(address, static_cast<int>(fileSize), &width, &height, &channelCount, componentCount);
		if (!pImageData)
			throw std::runtime_error(std::format("[ Texture ] ERROR\nFailed to load image data from the given address!\n"));

		return std::unique_ptr<uint8_t[]>(static_cast<uint8_t*>(pImageData));
	}

	VkImageView Texture::ImageView() const
	{
		return imageView;
	}

	VkImage Texture::Image() const
	{
		return imageMemory.Image();
	}

	const VkImageView* Texture::AddressOfImageView() const
	{
		return imageView.Address();
	}

	const VkImage* Texture::AddressOfImage() const
	{
		return imageMemory.AddressOfImage();
	}

	VkDescriptorImageInfo Texture::DescriptorImageInfo(VkSampler sampler) const
	{
		return VkDescriptorImageInfo{
			.sampler = sampler,
			.imageView = imageView,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};
	}

	std::unique_ptr<uint8_t[]> Texture::LoadFile(const char* filePath, VkExtent2D& extent, VkFormat format)
	{
		return LoadFile_Internal(filePath, 0, extent, format);
	}

	std::unique_ptr<uint8_t[]> Texture::LoadFile(const uint8_t* fileBinaries, size_t fileSize, VkExtent2D& extent, VkFormat format)
	{
		return LoadFile_Internal(fileBinaries, fileSize, extent, format);
	}

	void Texture::CopyBlitAndGenerateMipmap2D(VkBuffer buffer_copyFrom, VkImage image_copyTo, VkImage image_blitTo, VkExtent2D imageExtent, uint32_t mipLevelCount, uint32_t layerCount, VkFilter minFilter)
	{
		bool generateMipmap = mipLevelCount > 1;
		bool blitMipLevel0 = image_copyTo != image_blitTo;

		auto& commandBuffer = VulkanPlus::Plus().CommandBuffer_Transfer();
		commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkBufferImageCopy region = {
			.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount },
			.imageExtent = { imageExtent.width, imageExtent.height, 1 },
		};

		if (generateMipmap || blitMipLevel0)
			ImageUtils::CmdCopyBufferToImage(commandBuffer, buffer_copyFrom, image_copyTo, region,
				ImageBarrierInfo{ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
				ImageBarrierInfo{ VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL });
		else
			ImageUtils::CmdCopyBufferToImage(commandBuffer, buffer_copyFrom, image_copyTo, region,
				ImageBarrierInfo{ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
				ImageBarrierInfo{ VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL });

		if (blitMipLevel0) {
			VkImageBlit region = {
					{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount},
					{{},{int32_t(imageExtent.width), int32_t(imageExtent.height), 1}},
					{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount},
					{{},{int32_t(imageExtent.width), int32_t(imageExtent.height), 1}},
			};
			if (generateMipmap)
				ImageUtils::CmdBlitImage(commandBuffer, image_copyTo, image_blitTo, region,
					ImageBarrierInfo{ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
					ImageBarrierInfo{ VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL },
					minFilter);
			else
				ImageUtils::CmdBlitImage(commandBuffer, image_copyTo, image_blitTo, region,
					ImageBarrierInfo{ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
					ImageBarrierInfo{ VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
					minFilter);
		}

		if (generateMipmap)
			ImageUtils::CmdGenerateMipmap2D(commandBuffer, image_blitTo, imageExtent, mipLevelCount, layerCount,
				ImageBarrierInfo{ VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
				minFilter);

		commandBuffer.End();
		VulkanPlus::Plus().ExecuteCommandBuffer_Graphics(commandBuffer);
	}

	void Texture::BlitAndGenerateMipmap2D(VkImage image_preinitialized, VkImage image_final, VkExtent2D imageExtent, uint32_t mipLevelCount, uint32_t layerCount, VkFilter minFilter)
	{
		bool generateMipmap = mipLevelCount > 1;
		bool blitMipLevel0 = image_preinitialized != image_final;
		if (generateMipmap || blitMipLevel0) {
			auto& commandBuffer = VulkanPlus::Plus().CommandBuffer_Transfer();
			commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
			
			if (blitMipLevel0) {
				ImageUtils::CmdImagePipelineBarrier(commandBuffer, image_preinitialized,
					VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, layerCount },
					ImageBarrierInfo{ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_PREINITIALIZED },
					ImageBarrierInfo{ VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL }
				);

				VkImageBlit region = {
					{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount},
					{{},{int32_t(imageExtent.width), int32_t(imageExtent.height), 1}},
					{VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, layerCount},
					{{},{int32_t(imageExtent.width), int32_t(imageExtent.height), 1}},
				};
				if (generateMipmap)
					ImageUtils::CmdBlitImage(commandBuffer, image_preinitialized, image_final, region,
						ImageBarrierInfo{ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
						ImageBarrierInfo{ VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL },
						minFilter);
				else
					ImageUtils::CmdBlitImage(commandBuffer, image_preinitialized, image_final, region,
						ImageBarrierInfo{ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
						ImageBarrierInfo{ VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
						minFilter);
			}

			if (generateMipmap)
				ImageUtils::CmdGenerateMipmap2D(commandBuffer, image_final, imageExtent, mipLevelCount, layerCount,
					ImageBarrierInfo{ VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
					minFilter);

			commandBuffer.End();
			VulkanPlus::Plus().ExecuteCommandBuffer_Graphics(commandBuffer);
		}
	}

#pragma endregion

#pragma region Texture2D

	void Texture2D::Create_Internal(VkFormat initial_format, VkFormat final_format, bool generateMip)
	{
		mipLevelCount = generateMip ? ImageUtils::CalculateMipLevelCount(extent) : 1;
		CreateImageMemory(VK_IMAGE_TYPE_2D, final_format, { extent.width,extent.height,1 }, mipLevelCount, 1);
		CreateImageView(VK_IMAGE_VIEW_TYPE_2D, final_format, mipLevelCount, 1);

		imageViews.resize(mipLevelCount);
		for (uint32_t i = 0; i < mipLevelCount; i++)
		{
			imageViews[i].Create(imageMemory.Image(), VK_IMAGE_VIEW_TYPE_2D, final_format,
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, i, 1, 0, 1 }, VkComponentMapping{});
		}

		if (initial_format == final_format) 
			CopyBlitAndGenerateMipmap2D(StagingBuffer_MainThread::Main(), imageMemory.Image(), imageMemory.Image(), extent, mipLevelCount, 1);
		else {
			if (VkImage alisedImage = StagingBuffer_MainThread::AliasedImage2D(initial_format, extent))
				BlitAndGenerateMipmap2D(alisedImage, imageMemory.Image(), extent, mipLevelCount, 1);
			else {
				VkImageCreateInfo createInfo = {
					.imageType = VK_IMAGE_TYPE_2D,
					.format = initial_format,
					.extent = {extent.width, extent.height, 1},
					.mipLevels = mipLevelCount,
					.arrayLayers = 1,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.tiling = VK_IMAGE_TILING_LINEAR,
					.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
				};
				ImageMemory imageMemory_conversion(createInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				CopyBlitAndGenerateMipmap2D(StagingBuffer_MainThread::Main(), imageMemory_conversion.Image(), imageMemory.Image(), extent, mipLevelCount, 1);
			}
		}
	}

	Texture2D::Texture2D(const char* filePath, VkFormat initial_format, VkFormat final_format, bool generateMip)
	{
		Create(filePath, initial_format, final_format, generateMip);
	}

	Texture2D::Texture2D(const uint8_t* pImageData, VkExtent2D extent, VkFormat initial_format, VkFormat final_format, bool generateMip)
	{
		Create(pImageData, extent, initial_format, final_format, generateMip);
	}

	VkExtent2D Texture2D::Extent() const
	{
		return extent;
	}

	uint32_t Texture2D::Width() const
	{
		return extent.width;
	}

	uint32_t Texture2D::Height() const
	{
		return extent.height;
	}

	uint32_t Texture2D::MipLevelCount() const
	{
		return mipLevelCount;
	}


	VkDescriptorImageInfo Texture2D::DescriptorImageInfo(VkSampler sampler, uint32_t mipLevel) const
	{
		return VkDescriptorImageInfo{ sampler, imageViews[mipLevel], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	}

	void Texture2D::Create(const char* filePath, VkFormat initial_format, VkFormat final_format, bool generateMip)
	{
		VkExtent2D extent;
		std::unique_ptr<uint8_t[]> pImageData = LoadFile(filePath, extent, initial_format);
		if (pImageData)
			Create(pImageData.get(), extent, initial_format, final_format, generateMip);
	}

	void Texture2D::Create(const uint8_t* pImageData, VkExtent2D extent, VkFormat initial_format, VkFormat final_format, bool generateMip)
	{
		this->extent = extent;
		size_t imageDataSize = VkDeviceSize(vkuFormatElementSize(initial_format)) * extent.width * extent.height;
		StagingBuffer_MainThread::SynchronizeData(pImageData, imageDataSize);
		Create_Internal(initial_format, final_format, generateMip);
	}

#pragma endregion

#pragma region Attachment

	VkImageView Attachment::ImageView() const
	{
		return imageView;
	}

	VkImage Attachment::Image() const
	{
		return imageMemory.Image();
	}

	const VkImageView* Attachment::AddressOfImageView() const
	{
		return imageView.Address();
	}

	const VkImage* Attachment::AddressOfImage() const
	{
		return imageMemory.AddressOfImage();
	}

	VkDescriptorImageInfo Attachment::DescriptorImageInfo(VkSampler sampler) const
	{
		return VkDescriptorImageInfo{sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
	}

#pragma endregion


#pragma region ColorAttachment

	VkImageView ColorAttachment::ImageView(uint32_t mipLevel) const
	{
		if (mipLevel >= mipLevelCount)
			throw std::runtime_error("[ ColorAttachment ] ERROR\nMipLevel out of range!\n");
		return imageViews[mipLevel];
	}

	const VkImageView* ColorAttachment::AddressOfImageView(uint32_t mipLevel) const
	{
		if (mipLevel >= mipLevelCount)
			throw std::runtime_error("[ ColorAttachment ] ERROR\nMipLevel out of range!\n");
		return imageViews[mipLevel].Address();
	}

	uint32_t ColorAttachment::MipLevelCount() const
	{
		return mipLevelCount;
	}

	VkDescriptorImageInfo ColorAttachment::DescriptorImageInfo(VkSampler sampler, uint32_t mipLevel) const
	{
		if (mipLevel >= mipLevelCount)
			throw std::runtime_error("[ ColorAttachment ] ERROR\nMipLevel out of range!\n");
		return VkDescriptorImageInfo{ sampler, imageViews[mipLevel], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	}

	ColorAttachment::ColorAttachment(VkFormat format, VkExtent2D extent, bool hasMipmap, uint32_t layerCount, VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages)
	{
		Create(format, extent, hasMipmap, layerCount, sampleCount, otherUsages);
	}

	void ColorAttachment::Create(VkFormat format, VkExtent2D extent, bool hasMipmap, uint32_t layerCount, VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages)
	{
		mipLevelCount = hasMipmap ? ImageUtils::CalculateMipLevelCount(extent) : 1;
		VkImageCreateInfo imageCreateInfo = {
			.imageType = VK_IMAGE_TYPE_2D,
			.format = format,
			.extent = {extent.width, extent.height, 1},
			.mipLevels = mipLevelCount,
			.arrayLayers = layerCount,
			.samples = sampleCount,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | otherUsages,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		imageMemory.Create(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | bool(otherUsages & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) * VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);
		
		VkImageViewCreateInfo imageViewCreateInfo = {
			.image = imageMemory.Image(),
			.viewType = layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D,
			.format = format,
			.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevelCount, 0 ,layerCount}
		};
		imageView.Create(imageViewCreateInfo);

		imageViewCreateInfo.subresourceRange.levelCount = 1;
		for (uint32_t i = 0; i < mipLevelCount; i++) {
			imageViewCreateInfo.subresourceRange.baseMipLevel = i;
			imageViews.emplace_back(imageViewCreateInfo);
		}
	}

	bool ColorAttachment::FormatIsSupported(VkFormat format, bool supportBlending)
	{
		return ImageUtils::FormatProperties(format).optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT << uint32_t(supportBlending);
	}

#pragma endregion


#pragma region DepthStencilAttachment

	DepthStencilAttachment::DepthStencilAttachment(VkFormat format, VkExtent2D extent, bool stencilOnly, uint32_t layerCount, VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages)
	{
		Create(format, extent, stencilOnly, layerCount, sampleCount, otherUsages);
	}

	void DepthStencilAttachment::Create(VkFormat format, VkExtent2D extent, bool stencilOnly, uint32_t layerCount, VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages)
	{
		VkImageCreateInfo imageCreateInfo = {
			.imageType = VK_IMAGE_TYPE_2D,
			.format = format,
			.extent = { extent.width, extent.height, 1 },
			.mipLevels = 1,
			.arrayLayers = layerCount,
			.samples = sampleCount,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | otherUsages
		};
		imageMemory.Create(imageCreateInfo,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | bool(otherUsages & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) * VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);

		VkImageAspectFlags aspectMask = (!stencilOnly) * VK_IMAGE_ASPECT_DEPTH_BIT;
		if (format > VK_FORMAT_S8_UINT)
			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		else if (format == VK_FORMAT_S8_UINT)
			aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

		imageView.Create(imageMemory.Image(),
			layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, format,
			VkImageSubresourceRange{ aspectMask, 0, 1, 0, layerCount }, VkComponentMapping{});

	}

	bool DepthStencilAttachment::FormatIsSupported(VkFormat format)
	{
		return ImageUtils::FormatProperties(format).optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

#pragma endregion

	std::pair<int, std::span<Texture2D>> ImageManager::RecreateTexture2D(int id, const char* filePath, VkFormat initial_format, VkFormat final_format, bool generateMip)
	{
		if (auto it = mTexture2Ds.find(id); it != mTexture2Ds.end()) {
			it->second.~Texture2D();
			it->second.Create(filePath, initial_format, final_format, generateMip);
			return { id, std::span<Texture2D>(&it->second, 1) };
		}
		std::cerr << std::format("[ERROR] ImageManager: Texture2D with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<Texture2D>> ImageManager::RecreateTexture2D(int id, const uint8_t* pImageData, VkExtent2D extent, VkFormat initial_format, VkFormat final_format, bool generateMip)
	{
		if (auto it = mTexture2Ds.find(id); it != mTexture2Ds.end()) {
			it->second.~Texture2D();
			it->second.Create(pImageData, extent, initial_format, final_format, generateMip);
			return { id, std::span<Texture2D>(&it->second, 1) };
		}
		std::cerr << std::format("[ERROR] ImageManager: Texture2D with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<ColorAttachment>> ImageManager::RecreateColorAttachments(int id, uint32_t count, VkFormat format, VkExtent2D extent, bool hasMipmap, uint32_t layerCount, VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages)
	{
		if (count == 0) {
			std::cerr << std::format("[ERROR] ImageManager: '{}' requested 0 colorAttachments\n", id);
			return { M_INVALID_ID, {} };
		}

		if (auto it = mColorAttachments.find(id); it != mColorAttachments.end()) {
			it->second.clear();
			it->second.resize(count);
			for (size_t i = 0; i < it->second.size(); i++)
				it->second[i].Create(format, extent, hasMipmap, layerCount, sampleCount, otherUsages);
			return { id, std::span<ColorAttachment>(it->second.data(), it->second.size()) };
		}
		std::cerr << std::format("[ERROR] ImageManager: ColorAttachments with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<DepthStencilAttachment>> ImageManager::RecreateDepthStencilAttachments(int id, uint32_t count, VkFormat format, VkExtent2D extent, bool stencilOnly, uint32_t layerCount, VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages)
	{
		if (count == 0) {
			std::cerr << std::format("[ERROR] ImageManager: '{}' requested 0 depthStencilAttachments\n", id);
			return { M_INVALID_ID, {} };
		}

		if (auto it = mDepthStencilAttachments.find(id); it != mDepthStencilAttachments.end()) {
			it->second.clear();
			it->second.resize(count);
			for (size_t i = 0; i < it->second.size(); i++)
				it->second[i].Create(format, extent, stencilOnly, layerCount, sampleCount, otherUsages);
			return { id, std::span<DepthStencilAttachment>(it->second.data(), it->second.size()) };
		}
		std::cerr << std::format("[ERROR] ImageManager: DepthStencilAttachments with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<Texture2D>> ImageManager::CreateTexture2D(std::string name, const char* filePath, VkFormat initial_format, VkFormat final_format, bool generateMip)
	{
		if (auto it = mTexture2DIDs.find(name); it != mTexture2DIDs.end())
			return RecreateTexture2D(it->second, filePath, initial_format, final_format, generateMip);

		const int id = m_texture2d_id++;

		Texture2D texture(filePath, initial_format, final_format, generateMip);
		auto [it1, ok1] = mTexture2Ds.emplace(id, std::move(texture));
		if (!ok1) {
			std::cerr << std::format("[ERROR] ImageManager: Emplace texture2D for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mTexture2DIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] ImagePassManager: Texture2D '{}' has not been recorded!\n", name);
			mTexture2Ds.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<Texture2D>(&vec, 1) };
	}

	std::pair<int, std::span<Texture2D>> ImageManager::CreateTexture2D(std::string name, const uint8_t* pImageData, VkExtent2D extent, VkFormat initial_format, VkFormat final_format, bool generateMip)
	{
		if (auto it = mTexture2DIDs.find(name); it != mTexture2DIDs.end())
			return RecreateTexture2D(it->second, pImageData, extent, initial_format, final_format, generateMip);

		const int id = m_texture2d_id++;

		Texture2D texture(pImageData, extent, initial_format, final_format, generateMip);
		auto [it1, ok1] = mTexture2Ds.emplace(id, std::move(texture));
		if (!ok1) {
			std::cerr << std::format("[ERROR] ImageManager: Emplace texture2D for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mTexture2DIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] ImagePassManager: Texture2D '{}' has not been recorded!\n", name);
			mTexture2Ds.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<Texture2D>(&vec, 1) };
	}

	std::pair<int, std::span<Texture2D>> ImageManager::GetTexture2D(std::string name)
	{
		if (auto it = mTexture2DIDs.find(name); it != mTexture2DIDs.end())
			return GetTexture2D(it->second);
		std::cerr << std::format("[ERROR] ImageManager: Texture2D with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<Texture2D>> ImageManager::GetTexture2D(int id)
	{
		if (auto it = mTexture2Ds.find(id); it != mTexture2Ds.end())
			return { id, std::span<Texture2D>(&it->second, 1) };
		std::cerr << std::format("[ERROR] ImageManager: Texture2D with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	bool ImageManager::HasTexture2D(std::string name)
	{
		if (auto it = mTexture2DIDs.find(name); it != mTexture2DIDs.end())
			return HasTexture2D(it->second);
		return false;
	}

	bool ImageManager::HasTexture2D(int id)
	{
		return mTexture2Ds.contains(id);
	}

	size_t ImageManager::GetTexture2DCount() const
	{
		return mTexture2Ds.size();
	}

	std::pair<int, std::span<ColorAttachment>> ImageManager::CreateColorAttachments(std::string name, uint32_t count, VkFormat format, VkExtent2D extent, bool hasMipmap, uint32_t layerCount, VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages)
	{
		if (auto it = mColorAttachmentIDs.find(name); it != mColorAttachmentIDs.end())
			return RecreateColorAttachments(it->second, count, format, extent, hasMipmap, layerCount, sampleCount, otherUsages);

		if (count == 0) {
			std::cerr << std::format("[ERROR] ImageManager: '{}' requested 0 ColorAttachments\n", name);
			return { M_INVALID_ID, {} };
		}

		std::vector<ColorAttachment> ColorAttachments(count);
		for (size_t i = 0; i < ColorAttachments.size(); i++)
			ColorAttachments[i].Create(format, extent, hasMipmap, layerCount, sampleCount, otherUsages);

		const int id = m_color_attachment_id++;

		auto [it1, ok1] = mColorAttachments.emplace(id, std::move(ColorAttachments));
		if (!ok1) {
			std::cerr << std::format("[ERROR] ImageManager: Emplace colorAttachments for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mColorAttachmentIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] ImageManager: ColorAttachment '{}' has not been recorded!\n", name);
			mColorAttachments.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<ColorAttachment>(vec.data(), vec.size()) };
	}

	std::pair<int, std::span<ColorAttachment>> ImageManager::GetColorAttachments(std::string name)
	{
		if (auto it = mColorAttachmentIDs.find(name); it != mColorAttachmentIDs.end())
			return GetColorAttachments(it->second);
		std::cerr << std::format("[ERROR] ImageManager: ColorAttachments with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<ColorAttachment>> ImageManager::GetColorAttachments(int id)
	{
		if (auto it = mColorAttachments.find(id); it != mColorAttachments.end())
			return { id, std::span<ColorAttachment>(it->second.data(), it->second.size()) };
		std::cerr << std::format("[ERROR] ImageManager: ColorAttachments with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	int ImageManager::DestroyColorAttachments(std::string name)
	{
		if (auto it = mColorAttachmentIDs.find(name); it != mColorAttachmentIDs.end())
			return DestroyColorAttachments(it->second);
		std::cerr << std::format("[WARNING] ImageManager: ColorAttachments with name '{}' do not exist!\n", name);
		return M_INVALID_ID;
	}

	int ImageManager::DestroyColorAttachments(int id)
	{
		if (auto it = mColorAttachments.find(id); it != mColorAttachments.end()) {
			it->second.clear();
			return it->first;
		}
		std::cerr << std::format("[WARNING] ImageManager: ColorAttachments with id {} do not exist!\n", id);
		return M_INVALID_ID;
	}

	bool ImageManager::HasColorAttachments(std::string name)
	{
		if (auto it = mColorAttachmentIDs.find(name); it != mColorAttachmentIDs.end())
			return HasColorAttachments(it->second);
		return false;
	}

	bool ImageManager::HasColorAttachments(int id)
	{
		return mColorAttachments.contains(id);
	}

	size_t ImageManager::GetColorAttachmentsCount() const
	{
		return mColorAttachments.size();;
	}

	std::pair<int, std::span<DepthStencilAttachment>> ImageManager::CreateDepthStencilAttachments(std::string name, uint32_t count, VkFormat format, VkExtent2D extent, bool stencilOnly, uint32_t layerCount, VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages)
	{
		if (auto it = mDepthStencilAttachmentIDs.find(name); it != mDepthStencilAttachmentIDs.end())
			return RecreateDepthStencilAttachments(it->second, count, format, extent, stencilOnly, layerCount, sampleCount, otherUsages);

		if (count == 0) {
			std::cerr << std::format("[ERROR] ImageManager: '{}' requested 0 DepthStencilAttachments\n", name);
			return { M_INVALID_ID, {} };
		}

		std::vector<DepthStencilAttachment> DepthStencilAttachments(count);
		for (size_t i = 0; i < DepthStencilAttachments.size(); i++)
			DepthStencilAttachments[i].Create(format, extent, stencilOnly, layerCount, sampleCount, otherUsages);

		const int id = m_depth_stencil_attachment_id++;

		auto [it1, ok1] = mDepthStencilAttachments.emplace(id, std::move(DepthStencilAttachments));
		if (!ok1) {
			std::cerr << std::format("[ERROR] ImageManager: Emplace depthStencilAttachments for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mDepthStencilAttachmentIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] ImageManager: DepthStencilAttachment '{}' has not been recorded!\n", name);
			mDepthStencilAttachments.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<DepthStencilAttachment>(vec.data(), vec.size()) };
	}

	std::pair<int, std::span<DepthStencilAttachment>> ImageManager::GetDepthStencilAttachments(std::string name)
	{
		if (auto it = mDepthStencilAttachmentIDs.find(name); it != mDepthStencilAttachmentIDs.end())
			return GetDepthStencilAttachments(it->second);
		std::cerr << std::format("[ERROR] ImageManager: DepthStencilAttachments with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<DepthStencilAttachment>> ImageManager::GetDepthStencilAttachments(int id)
	{
		if (auto it = mDepthStencilAttachments.find(id); it != mDepthStencilAttachments.end())
			return { id, std::span<DepthStencilAttachment>(it->second.data(), it->second.size()) };
		std::cerr << std::format("[ERROR] ImageManager: DepthStencilAttachments with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	int ImageManager::DestroyDepthStencilAttachments(std::string name)
	{
		if (auto it = mDepthStencilAttachmentIDs.find(name); it != mDepthStencilAttachmentIDs.end())
			return DestroyDepthStencilAttachments(it->second);
		std::cerr << std::format("[WARNING] ImageManager: DepthStencilAttachments with name '{}' do not exist!\n", name);
		return M_INVALID_ID;
	}

	int ImageManager::DestroyDepthStencilAttachments(int id)
	{
		if (auto it = mDepthStencilAttachments.find(id); it != mDepthStencilAttachments.end()) {
			it->second.clear();
			return it->first;
		}
		std::cerr << std::format("[WARNING] ImageManager: DepthStencilAttachments with id {} do not exist!\n", id);
		return M_INVALID_ID;
	}

	bool ImageManager::HasDepthStencilAttachments(std::string name)
	{
		if (auto it = mDepthStencilAttachmentIDs.find(name); it != mDepthStencilAttachmentIDs.end())
			return HasDepthStencilAttachments(it->second);
		return false;
	}

	bool ImageManager::HasDepthStencilAttachments(int id)
	{
		return mDepthStencilAttachments.contains(id);
	}

	size_t ImageManager::GetDepthStencilAttachmentsCount() const
	{
		return mDepthStencilAttachments.size();;
	}

}