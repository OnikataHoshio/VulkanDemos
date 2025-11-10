#include "Plus/ImageManager.h"
#include "Plus/VulkanPlus.h"
#include "Utils/ImageUtils.h"

namespace HoshioEngine {

#pragma region Texture

	void Texture::CreateImageMemory(VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t mipLevelCount, uint32_t arrayLayerCount, VkImageCreateFlags flags)
	{
		VkImageCreateInfo createInfo = {
			.flags = flags,
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

	void TextureArray::Create_Internal(VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		uint32_t mipLevelCount = generateMipmap ? ImageUtils::CalculateMipLevelCount(extent) : 1;
		CreateImageMemory(VK_IMAGE_TYPE_2D, format_final, { extent.width, extent.height, 1 }, mipLevelCount, layerCount);
		CreateImageView(VK_IMAGE_VIEW_TYPE_2D_ARRAY, format_final, mipLevelCount, layerCount);
		if (format_initial == format_final)
			CopyBlitAndGenerateMipmap2D(StagingBuffer_MainThread::Main(), imageMemory.Image(), imageMemory.Image(), extent, mipLevelCount, layerCount);
		else {
			VkImageCreateInfo createInfo = {
				.imageType = VK_IMAGE_TYPE_2D,
				.format = format_initial,
				.extent = { extent.width, extent.height, 1 },
				.mipLevels = 1,
				.arrayLayers = layerCount,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
			};
			ImageMemory imageMemory_conversion(createInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			CopyBlitAndGenerateMipmap2D(StagingBuffer_MainThread::Main(), imageMemory_conversion.Image(), imageMemory.Image(), extent, mipLevelCount, 1);
		}
	}

	TextureArray::TextureArray(const char* filepath, VkExtent2D extentInTiles, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		Create(filepath, extentInTiles, format_initial, format_final, generateMipmap);
	}

	TextureArray::TextureArray(const uint8_t* pImageData, VkExtent2D fullExtent, VkExtent2D extentInTiles, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		Create(pImageData, fullExtent, extentInTiles, format_initial, format_final, generateMipmap);
	}

	TextureArray::TextureArray(ArrayRef<const char* const> filepaths, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		Create(filepaths, format_initial, format_final, generateMipmap);
	}

	TextureArray::TextureArray(ArrayRef<const uint8_t* const> psImageData, VkExtent2D extent, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		Create(psImageData, extent, format_initial, format_final, generateMipmap);
	}

	VkExtent2D TextureArray::Extent() const
	{
		return extent;
	}

	uint32_t TextureArray::Width() const
	{
		return extent.width;
	}

	uint32_t TextureArray::Height() const
	{
		return extent.height;
	}

	uint32_t TextureArray::LayerCount() const
	{
		return layerCount;
	}

	void TextureArray::Create(const char* filepath, VkExtent2D extentInTiles, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		if (extentInTiles.width * extentInTiles.height > VulkanBase::Base().PhysicalDeviceProperties().limits.maxImageArrayLayers) {
			std::cerr << std::format("[ TextureArray ] ERROR\nLayer count is out of limit! Must be less than: {}\nFile: {}\n", VulkanBase::Base().PhysicalDeviceProperties().limits.maxImageArrayLayers, filepath);
			throw std::runtime_error("[ TextureArray ] ERROR::Layer count is out of limit!");
		}
		VkExtent2D fullExtent;
		std::unique_ptr<uint8_t[]> pImageData = LoadFile(filepath, fullExtent, format_initial);
		if (pImageData) {
			if (fullExtent.width % extentInTiles.width ||
				fullExtent.height % extentInTiles.height) {
				std::cerr << std::format(
					"[ TextureArray ] ERROR\nImage not available!\nFile: {}\nImage width must be in multiples of {}\nImage height must be in multiples of {}\n",
					filepath, extentInTiles.width, extentInTiles.height);
				throw std::runtime_error("[ TextureArray ] ERROR::Image width must be in multiples !");
			}
			else
				Create(pImageData.get(), fullExtent, extentInTiles, format_initial, format_final, generateMipmap);
		}

	}

	void TextureArray::Create(const uint8_t* pImageData, VkExtent2D fullExtent, VkExtent2D extentInTiles, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		layerCount = extentInTiles.width * extentInTiles.height;
		if (layerCount > VulkanBase::Base().PhysicalDeviceProperties().limits.maxImageArrayLayers) {
			std::cerr << std::format("[ TextureArray ] ERROR\nLayer count is out of limit! Must be less than: {}\n", VulkanBase::Base().PhysicalDeviceProperties().limits.maxImageArrayLayers);
			throw std::runtime_error("[ TextureArray ] ERROR::Layer count is out of limit!");
		}
		if (fullExtent.width % extentInTiles.width ||
			fullExtent.height % extentInTiles.height) {
			std::cerr << std::format(
				"[ TextureArray ] ERROR\nImage not available!\nImage width must be in multiples of {}\nImage height must be in multiples of {}\n",
				extentInTiles.width, extentInTiles.height);
			throw std::runtime_error("[ TextureArray ] ERROR::Image width must be in multiples !");
		}

		extent.width = fullExtent.width / extentInTiles.width;
		extent.height = fullExtent.height / extentInTiles.height;

		size_t dataSizePerPixel = vkuFormatElementSize(format_initial);
		size_t imageDataSize = dataSizePerPixel * fullExtent.width * fullExtent.height;

		if (extentInTiles.width == 1)
			StagingBuffer_MainThread::SynchronizeData(pImageData, imageDataSize);
		else {
			uint8_t* pData_dst = static_cast<uint8_t*>(StagingBuffer_MainThread::MapMemory(imageDataSize));
			size_t dataSizePerRow = dataSizePerPixel * extent.width;
			for (size_t j = 0; j < extentInTiles.height; j++)
				for (size_t i = 0; i < extentInTiles.width; i++) {
					for (size_t k = 0; k < extent.height; k++)
						memcpy(
							pData_dst,
							pImageData + (i * extent.width + (k + j * extent.height) * fullExtent.width) * dataSizePerPixel,
							dataSizePerRow),
						pData_dst += dataSizePerRow; //每拷贝一行，pData_dst向后移动一行的数据大小
				}
			StagingBuffer_MainThread::UnMapMemory();
		}
		Create_Internal(format_initial, format_final, generateMipmap);
	}

	void TextureArray::Create(ArrayRef<const char* const> filepaths, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		if (filepaths.size() > VulkanBase::Base().PhysicalDeviceProperties().limits.maxImageArrayLayers) {
			std::cerr << std::format(
				"[ TextureArray ] ERROR\nLayer count is out of limit! Must be less than: {}\n",
				VulkanBase::Base().PhysicalDeviceProperties().limits.maxImageArrayLayers);
			throw std::runtime_error("[ TextureArray ] ERROR::Layer count is out of limit!");
		}
		std::unique_ptr psImageData = std::make_unique<std::unique_ptr<uint8_t[]>[]>(filepaths.size());
		for (size_t i = 0; i < filepaths.size(); i++) {
			VkExtent2D extent_currentLayer;
			psImageData[i] = LoadFile(filepaths[i], extent_currentLayer, format_initial);
			if (psImageData[i]) {
				if (i == 0)
					extent = extent_currentLayer;
				if (extent.width == extent_currentLayer.width &&
					extent.height == extent_currentLayer.height)
					continue;
				else {
					std::cerr << std::format(
						"[ TextureArray ] ERROR\nImage not available!\nFile: {}\nAll the images must be in same size!\n",
						filepaths[i]);//fallthrough
					throw std::runtime_error("[ TextureArray ] All the images must be in same size!");
				}
			}
			return;
		}
		Create({ reinterpret_cast<const uint8_t* const*>(psImageData.get()), filepaths.size() }, extent, format_initial, format_final, generateMipmap);
	}

	void TextureArray::Create(ArrayRef<const uint8_t* const> psImageData, VkExtent2D extent, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		layerCount = psImageData.size();
		if (layerCount > VulkanBase::Base().PhysicalDeviceProperties().limits.maxImageArrayLayers) {
			std::cerr << std::format(
				"[ TextureArray ] ERROR\nLayer count is out of limit! Must be less than: {}\n",
				VulkanBase::Base().PhysicalDeviceProperties().limits.maxImageArrayLayers);
				throw std::runtime_error("[ TextureArray ] ERROR::Layer count is out of limit!");
		}
		this->extent = extent;
		size_t dataSizePerImage = vkuFormatElementSize(format_initial) * extent.width * extent.height;
		size_t imageDataSize = dataSizePerImage * layerCount;
		uint8_t* pData_dst = static_cast<uint8_t*>(StagingBuffer_MainThread::MapMemory(imageDataSize));
		for (size_t i = 0; i < layerCount; i++)
			memcpy(pData_dst, psImageData[i], dataSizePerImage),
			pData_dst += dataSizePerImage;
		StagingBuffer_MainThread::UnMapMemory();
		//Create image and allocate memory, create image view, then copy data from staging buffer to image
		Create_Internal(format_initial, format_final, generateMipmap);
	}

	VkExtent2D TextureCube::GetExtentInTiles(const glm::uvec2*& facePositions, bool lookFromOutside, bool loadPreviousResult)
	{
		static constexpr glm::uvec2 facePositions_default[][6] = {
			{ { 2, 1 }, { 0, 1 }, { 1, 0 }, { 1, 2 }, { 3, 1 }, { 1, 1 } },
			{ { 2, 1 }, { 0, 1 }, { 1, 0 }, { 1, 2 }, { 1, 1 }, { 3, 1 } }
		};
		static VkExtent2D extentInTiles;
		if (loadPreviousResult)
			return extentInTiles;
		extentInTiles = { 1, 1 };
		if (!facePositions)
			facePositions = facePositions_default[lookFromOutside],
			extentInTiles = { 4, 3 };
		else
			for (size_t i = 0; i < 6; i++) {
				if (facePositions[i].x >= extentInTiles.width)
					extentInTiles.width = facePositions[i].x + 1;
				if (facePositions[i].y >= extentInTiles.height)
					extentInTiles.height = facePositions[i].y + 1;
			}
		return extentInTiles;
	}

	void TextureCube::Create_Internal(VkFormat format_initial, VkFormat format_final, bool generateMip) {
		uint32_t mipLevelCount = generateMip ? ImageUtils::CalculateMipLevelCount(extent) : 1;
		CreateImageMemory(VK_IMAGE_TYPE_2D, format_final, { extent.width, extent.height, 1 }, mipLevelCount, 6, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
		CreateImageView(VK_IMAGE_VIEW_TYPE_CUBE, format_final, mipLevelCount, 6);
		if (format_initial == format_final)
			CopyBlitAndGenerateMipmap2D(StagingBuffer_MainThread::Main(), imageMemory.Image(), imageMemory.Image(), extent, mipLevelCount, 6);
		else {
			VkImageCreateInfo createInfo = {
				.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
				.imageType = VK_IMAGE_TYPE_2D,
				.format = format_initial,
				.extent = { extent.width, extent.height, 1 },
				.mipLevels = 1,
				.arrayLayers = 6,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
			};
			ImageMemory imageMemory_conversion(createInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			CopyBlitAndGenerateMipmap2D(StagingBuffer_MainThread::Main(), imageMemory_conversion.Image(), imageMemory.Image(), extent, mipLevelCount, 6);
		}
	}

	TextureCube::TextureCube(const char* filepath, const glm::uvec2 facePositions[6], VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap) {
		Create(filepath, facePositions, format_initial, format_final, lookFromOutside, generateMipmap);
	}

	TextureCube::TextureCube(const uint8_t* pImageData, VkExtent2D fullExtent, const glm::uvec2 facePositions[6], VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap)
	{
		Create(pImageData, fullExtent, facePositions, format_initial, format_final, lookFromOutside, generateMipmap);
	}

	TextureCube::TextureCube(const char* const* filepaths, VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap)
	{
		Create(filepaths, format_initial, format_final, lookFromOutside, generateMipmap);
	}

	TextureCube::TextureCube(const uint8_t* const* psImageData, VkExtent2D extent, VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap)
	{
		Create(psImageData, extent, format_initial, format_final, lookFromOutside, generateMipmap);
	}

	VkExtent2D TextureCube::Extent() const
	{
		return extent;
	}

	uint32_t TextureCube::Width() const
	{
		return extent.width;
	}

	uint32_t TextureCube::Height() const
	{
		return extent.height;
	}
	void TextureCube::Create(const char* filepath, const glm::uvec2 facePositions[6], VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap)
	{
		VkExtent2D fullExtent;
		std::unique_ptr<uint8_t[]> pImageData = LoadFile(filepath, fullExtent, format_initial);
		if (pImageData) {
			if (VkExtent2D extentInTiles = GetExtentInTiles(facePositions, lookFromOutside);
				fullExtent.width % extentInTiles.width ||
				fullExtent.height % extentInTiles.height){
				std::cerr << std::format("[TextureCube] ERROR\nImage not available!\nFile: {}\nImage width should be in multiples of{}\nImage height should be in multiples of{}\n",
					filepath, extentInTiles.width, extentInTiles.height);
				throw std::runtime_error("[TextureCube] ERROR ::Image width should be in multiples");
				}
			else {
				extent.width = fullExtent.width / extentInTiles.width;
				extent.height = fullExtent.height / extentInTiles.height;
				Create(pImageData.get(), { fullExtent.width, UINT32_MAX }, facePositions, format_initial, format_final, lookFromOutside, generateMipmap);
			}
		}
	}

	void TextureCube::Create(const uint8_t* pImageData, VkExtent2D fullExtent, const glm::uvec2 facePositions[6], VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap)
	{
		VkExtent2D extentInTiles;
		if (fullExtent.height == UINT32_MAX)
			extentInTiles = GetExtentInTiles(facePositions, lookFromOutside, true);
		else {
			extentInTiles = GetExtentInTiles(facePositions, lookFromOutside);
			if (fullExtent.width % extentInTiles.width ||
				fullExtent.height % extentInTiles.height) {
				std::cerr << std::format("[TextureCube] ERROR\nImage not available!\nImage width should be in multiples of{}\nImage height should be in multiples of{}\n",
					extentInTiles.width, extentInTiles.height);
				throw std::runtime_error("[TextureCube] ERROR ::Image width should be in multiples");
			}
			extent.width = fullExtent.width / extentInTiles.width;
			extent.height = fullExtent.height / extentInTiles.height;
		}

		size_t dataSizePerPixel = vkuFormatElementSize(format_initial);
		size_t dataSizePerRow = dataSizePerPixel * extent.width;
		size_t dataSizePerImage = dataSizePerRow * extent.height;
		size_t imageDataSize = dataSizePerImage * 6;
		uint8_t* pData_dst = static_cast<uint8_t*>(StagingBuffer_MainThread::MapMemory(imageDataSize));

		if (lookFromOutside) {
			if (extentInTiles.width == 1 && extentInTiles.height == 6 &&
				facePositions[0].y == 0 && facePositions[1].y == 1 &&
				facePositions[2].y == 2 && facePositions[3].y == 3 &&
				facePositions[4].y == 4 && facePositions[5].y == 5)
				memcpy(pData_dst, pImageData, imageDataSize);
			else
				for (size_t face = 0; face < 6; face++)
					for (uint32_t j = 0; j < extent.height; j++)
						memcpy(
							pData_dst,
							pImageData + dataSizePerPixel * (facePositions[face].x * extent.width + (j + facePositions[face].y * extent.height) * fullExtent.width),
							dataSizePerRow),
						pData_dst += dataSizePerRow;
		}
		else {
			for (size_t face = 0; face < 6; face++) {
				if (face != 2 && face != 3)
					for (uint32_t i = 0; i < extent.height; i++)
						for (uint32_t j = 0; j < extent.width; j++)
							memcpy(
								pData_dst,
								pImageData + dataSizePerPixel * (extent.width - 1 - j + facePositions[face].x * extent.width + (i + facePositions[face].y * extent.height) * fullExtent.width),
								dataSizePerPixel),
							pData_dst += dataSizePerPixel;
				else
					for (uint32_t j = 0; j < extent.height; j++)
						for (uint32_t k = 0; k < extent.width; k++)
							memcpy(
								pData_dst,
								pImageData + dataSizePerPixel * (k + facePositions[face].x * extent.width + ((extent.height - 1 - j) + facePositions[face].y * extent.height) * fullExtent.width),
								dataSizePerPixel),
							pData_dst += dataSizePerPixel;
			}
		}
		StagingBuffer_MainThread::UnMapMemory();
		Create_Internal(format_initial, format_final, generateMipmap);
	}

	void TextureCube::Create(const char* const* filepaths, VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap)
	{
		std::unique_ptr<uint8_t[]> psImageData[6] = {};
		for (size_t i = 0; i < 6; i++) {
			VkExtent2D extent_currentLayer;
			psImageData[i] = LoadFile(filepaths[i], extent_currentLayer, format_initial);
			if (psImageData[i]) {
				if (i == 0)
					extent = extent_currentLayer;
				if (extent.width == extent_currentLayer.width ||
					extent.height == extent_currentLayer.height)
					continue;
				else {
					std::cerr << std::format("[ textureCube ] ERROR\nImage not available!\nFile: {}\nAll the images must be in same size!\n", filepaths[i]);
					throw std::runtime_error("[ textureCube ] ERROR::Image not available!");
				}
			}
			return;
		}
		Create(reinterpret_cast<const uint8_t* const*>(psImageData), extent, format_initial, format_final, lookFromOutside, generateMipmap);
	}

	void TextureCube::Create(const uint8_t* const* psImageData, VkExtent2D extent, VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap)
	{
		this->extent = extent;
		size_t dataSizePerPixel = vkuFormatElementSize(format_initial);
		size_t dataSizePerImage = dataSizePerPixel * extent.width * extent.height;
		size_t imageDataSize = dataSizePerImage * 6;
		uint8_t* pData_dst = static_cast<uint8_t*>(StagingBuffer_MainThread::MapMemory(imageDataSize));
		if (lookFromOutside) {
			for (size_t i = 0; i < 6; i++)
				memcpy(pData_dst + dataSizePerImage * i, psImageData[i], dataSizePerImage);
		}
		else {
			for (size_t face = 0; face < 6; face++) {
				if (face != 2 && face != 3)
					for (uint32_t j = 0; j < extent.height; j++)
						for (uint32_t i = 0; i < extent.width; i++)
							memcpy(
								pData_dst,
								psImageData[face] + dataSizePerPixel * ((j + 1) * extent.width - 1 - i),
								dataSizePerPixel),
							pData_dst += dataSizePerPixel;
				else
					for (uint32_t j = 0; j < extent.height; j++)
						for (uint32_t i = 0; i < extent.width; i++)
							memcpy(
								pData_dst,
								psImageData[face] + dataSizePerPixel * ((extent.height - 1 - j) * extent.width + i),
								dataSizePerPixel),
							pData_dst += dataSizePerPixel;
			}
		}
		StagingBuffer_MainThread::UnMapMemory();
		Create_Internal(format_initial, format_final, generateMipmap);
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


#pragma region CubeAttachment
	uint32_t HoshioEngine::CubeAttachment::MipLevelCount() const
	{
		return mipLevelCount;
	}

	CubeAttachment::CubeAttachment(VkFormat format, VkExtent2D extent, bool hasMipmap, VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages)
	{
		Create(format, extent, hasMipmap, sampleCount, otherUsages);
	}

	void CubeAttachment::Create(VkFormat format, VkExtent2D extent, bool hasMipmap, VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages)
	{
		mipLevelCount = hasMipmap ? ImageUtils::CalculateMipLevelCount(extent) : 1;
		VkImageCreateInfo imageCreateInfo = {
			.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = format,
			.extent = {extent.width, extent.height, 1},
			.mipLevels = mipLevelCount,
			.arrayLayers = 6,
			.samples = sampleCount,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | otherUsages,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};

		imageMemory.Create(imageCreateInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | bool(otherUsages & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) * VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);

		VkImageViewCreateInfo imageViewCreateInfo = {
			.image = imageMemory.Image(),
			.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
			.format = format,
			.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevelCount, 0 , 6}
		};
		imageView.Create(imageViewCreateInfo);
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

	std::pair<int, std::span<CubeAttachment>> ImageManager::RecreateCubeAttachments(int id, uint32_t count, VkFormat format, VkExtent2D extent, bool hasMipmap,  VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages)
	{
		if (count == 0) {
			std::cerr << std::format("[ERROR] ImageManager: '{}' requested 0 CubeAttachments\n", id);
			return { M_INVALID_ID, {} };
		}

		if (auto it = mCubeAttachments.find(id); it != mCubeAttachments.end()) {
			it->second.clear();
			it->second.resize(count);
			for (size_t i = 0; i < it->second.size(); i++)
				it->second[i].Create(format, extent, hasMipmap, sampleCount, otherUsages);
			return { id, std::span<CubeAttachment>(it->second.data(), it->second.size()) };
		}
		std::cerr << std::format("[ERROR] ImageManager: CubeAttachments with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<Texture2D>> ImageManager::CreateTexture2D(std::string name, const char* filePath, VkFormat initial_format, VkFormat final_format, bool generateMip)
	{
		if (auto it = mTexture2DIDs.find(name); it != mTexture2DIDs.end()) {
			std::cout << std::format("[ImageManager]::Warning::Texture({}) has been loaded!", name);
			return GetTexture2D(name);
		}

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
		if (auto it = mTexture2DIDs.find(name); it != mTexture2DIDs.end()) {
			std::cout << std::format("[ImageManager]::Warning::Texture({}) has been loaded!", name);
			return GetTexture2D(name);
		}

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

	std::pair<int, std::span<TextureArray>> ImageManager::CreateTextureArray(std::string name, const char* filepath, VkExtent2D extentInTiles, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		if (auto it = mTextureArrayIDs.find(name); it != mTextureArrayIDs.end()) {
			std::cout << std::format("[ImageManager]::Warning::TextureArray({}) has been loaded!", name);
			return GetTextureArray(name);
		}

		const int id = m_texture_array_id++;

		TextureArray texture(filepath, extentInTiles, format_initial, format_final, generateMipmap);
		auto [it1, ok1] = mTextureArrays.emplace(id, std::move(texture));
		if (!ok1) {
			std::cerr << std::format("[ERROR] ImageManager: Emplace TextureArray for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mTextureArrayIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] ImagePassManager: TextureArray '{}' has not been recorded!\n", name);
			mTextureArrays.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<TextureArray>(&vec, 1) };
	}

	std::pair<int, std::span<TextureArray>> ImageManager::CreateTextureArray(std::string name, const uint8_t* pImageData, VkExtent2D fullExtent, VkExtent2D extentInTiles, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		if (auto it = mTextureArrayIDs.find(name); it != mTextureArrayIDs.end()) {
			std::cout << std::format("[ImageManager]::Warning::TextureArray({}) has been loaded!", name);
			return GetTextureArray(name);
		}

		const int id = m_texture_array_id++;

		TextureArray texture(pImageData, fullExtent, extentInTiles, format_initial, format_final, generateMipmap);
		auto [it1, ok1] = mTextureArrays.emplace(id, std::move(texture));
		if (!ok1) {
			std::cerr << std::format("[ERROR] ImageManager: Emplace TextureArray for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mTextureArrayIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] ImagePassManager: TextureArray '{}' has not been recorded!\n", name);
			mTextureArrays.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<TextureArray>(&vec, 1) };
	}

	std::pair<int, std::span<TextureArray>> ImageManager::CreateTextureArray(std::string name, ArrayRef<const char* const> filepaths, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		if (auto it = mTextureArrayIDs.find(name); it != mTextureArrayIDs.end()) {
			std::cout << std::format("[ImageManager]::Warning::TextureArray({}) has been loaded!", name);
			return GetTextureArray(name);
		}

		const int id = m_texture_array_id++;

		TextureArray texture(filepaths, format_initial, format_final, generateMipmap);
		auto [it1, ok1] = mTextureArrays.emplace(id, std::move(texture));
		if (!ok1) {
			std::cerr << std::format("[ERROR] ImageManager: Emplace TextureArray for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mTextureArrayIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] ImagePassManager: TextureArray '{}' has not been recorded!\n", name);
			mTextureArrays.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<TextureArray>(&vec, 1) };
	}

	std::pair<int, std::span<TextureArray>> ImageManager::CreateTextureArray(std::string name, ArrayRef<const uint8_t* const> psImageData, VkExtent2D extent, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		if (auto it = mTextureArrayIDs.find(name); it != mTextureArrayIDs.end()) {
			std::cout << std::format("[ImageManager]::Warning::TextureArray({}) has been loaded!", name);
			return GetTextureArray(name);
		}

		const int id = m_texture_array_id++;

		TextureArray texture(psImageData, extent, format_initial, format_final, generateMipmap);
		auto [it1, ok1] = mTextureArrays.emplace(id, std::move(texture));
		if (!ok1) {
			std::cerr << std::format("[ERROR] ImageManager: Emplace TextureArray for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mTextureArrayIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] ImagePassManager: TextureArray '{}' has not been recorded!\n", name);
			mTextureArrays.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<TextureArray>(&vec, 1) };
	}

	std::pair<int, std::span<TextureArray>> ImageManager::GetTextureArray(std::string name)
	{
		if (auto it = mTextureArrayIDs.find(name); it != mTextureArrayIDs.end())
			return GetTextureArray(it->second);
		std::cerr << std::format("[ERROR] ImageManager: TextureArray with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<TextureArray>> ImageManager::GetTextureArray(int id)
	{
		if (auto it = mTextureArrays.find(id); it != mTextureArrays.end())
			return { id, std::span<TextureArray>(&it->second, 1) };
		std::cerr << std::format("[ERROR] ImageManager: TextureArray with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	bool ImageManager::HasTextureArray(std::string name)
	{
		if (auto it = mTextureArrayIDs.find(name); it != mTextureArrayIDs.end())
			return HasTexture2D(it->second);
		return false;
	}

	bool ImageManager::HasTextureArray(int id)
	{
		return mTextureArrays.contains(id);
	}

	size_t ImageManager::GetTextureArrayCount() const
	{
		return mTextureArrays.size();
	}

	std::pair<int, std::span<TextureCube>> ImageManager::CreateTextureCube(std::string name, const char* filepath, const glm::uvec2 facePositions[6], VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap)
	{
		if (auto it = mTextureCubeIDs.find(name); it != mTextureCubeIDs.end()) {
			std::cout << std::format("[ImageManager]::Warning::TextureCube({}) has been loaded!", name);
			return GetTextureCube(name);
		}

		const int id = m_texture_cube_id++;

		TextureCube texture(filepath, facePositions, format_initial, format_final, lookFromOutside, generateMipmap);
		auto [it1, ok1] = mTextureCubes.emplace(id, std::move(texture));
		if (!ok1) {
			std::cerr << std::format("[ERROR] ImageManager: Emplace TextureCube for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mTextureCubeIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] ImagePassManager: TextureCube '{}' has not been recorded!\n", name);
			mTextureCubes.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<TextureCube>(&vec, 1) };
	}

	std::pair<int, std::span<TextureCube>> ImageManager::CreateTextureCube(std::string name, const uint8_t* pImageData, VkExtent2D fullExtent, const glm::uvec2 facePositions[6], VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap)
	{
		if (auto it = mTextureCubeIDs.find(name); it != mTextureCubeIDs.end()) {
			std::cout << std::format("[ImageManager]::Warning::TextureCube({}) has been loaded!", name);
			return GetTextureCube(name);
		}

		const int id = m_texture_cube_id++;

		TextureCube texture(pImageData, fullExtent, facePositions, format_initial, format_final, lookFromOutside, generateMipmap);
		auto [it1, ok1] = mTextureCubes.emplace(id, std::move(texture));
		if (!ok1) {
			std::cerr << std::format("[ERROR] ImageManager: Emplace TextureCube for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mTextureCubeIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] ImagePassManager: TextureCube '{}' has not been recorded!\n", name);
			mTextureCubes.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<TextureCube>(&vec, 1) };
	}

	std::pair<int, std::span<TextureCube>> ImageManager::CreateTextureCube(std::string name, const char* const* filepaths, VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap)
	{
		if (auto it = mTextureCubeIDs.find(name); it != mTextureCubeIDs.end()) {
			std::cout << std::format("[ImageManager]::Warning::TextureCube({}) has been loaded!", name);
			return GetTextureCube(name);
		}

		const int id = m_texture_cube_id++;

		TextureCube texture(filepaths, format_initial, format_final, lookFromOutside, generateMipmap);
		auto [it1, ok1] = mTextureCubes.emplace(id, std::move(texture));
		if (!ok1) {
			std::cerr << std::format("[ERROR] ImageManager: Emplace TextureCube for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mTextureCubeIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] ImagePassManager: TextureCube '{}' has not been recorded!\n", name);
			mTextureCubes.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<TextureCube>(&vec, 1) };
	}

	std::pair<int, std::span<TextureCube>> ImageManager::CreateTextureCube(std::string name, const uint8_t* const* psImageData, VkExtent2D extent, VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap)
	{
		if (auto it = mTextureCubeIDs.find(name); it != mTextureCubeIDs.end()) {
			std::cout << std::format("[ImageManager]::Warning::TextureCube({}) has been loaded!", name);
			return GetTextureCube(name);
		}

		const int id = m_texture_cube_id++;

		TextureCube texture(psImageData, extent, format_initial, format_final, lookFromOutside, generateMipmap);
		auto [it1, ok1] = mTextureCubes.emplace(id, std::move(texture));
		if (!ok1) {
			std::cerr << std::format("[ERROR] ImageManager: Emplace TextureCube for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mTextureCubeIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] ImagePassManager: TextureCube '{}' has not been recorded!\n", name);
			mTextureCubes.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<TextureCube>(&vec, 1) };
	}

	std::pair<int, std::span<TextureCube>> ImageManager::GetTextureCube(std::string name)
	{
		if (auto it = mTextureCubeIDs.find(name); it != mTextureCubeIDs.end())
			return GetTextureCube(it->second);
		std::cerr << std::format("[ERROR] ImageManager: TextureCube with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<TextureCube>> ImageManager::GetTextureCube(int id)
	{
		if (auto it = mTextureCubes.find(id); it != mTextureCubes.end())
			return { id, std::span<TextureCube>(&it->second, 1) };
		std::cerr << std::format("[ERROR] ImageManager: TextureCube with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	bool ImageManager::HasTextureCube(std::string name)
	{
		if (auto it = mTextureCubeIDs.find(name); it != mTextureCubeIDs.end())
			return HasTexture2D(it->second);
		return false;
	}

	bool ImageManager::HasTextureCube(int id)
	{
		return mTextureCubes.contains(id);
	}

	size_t ImageManager::GetTextureCubeCount() const
	{
		return mTextureCubes.size();
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

	std::pair<int, std::span<CubeAttachment>> ImageManager::CreateCubeAttachments(std::string name, uint32_t count, VkFormat format, VkExtent2D extent, bool hasMipmap, VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages)
	{
		if (auto it = mCubeAttachmentIDs.find(name); it != mCubeAttachmentIDs.end())
			return RecreateCubeAttachments(it->second, count, format, extent, hasMipmap, sampleCount, otherUsages);

		if (count == 0) {
			std::cerr << std::format("[ERROR] ImageManager: '{}' requested 0 CubeAttachments\n", name);
			return { M_INVALID_ID, {} };
		}

		std::vector<CubeAttachment> CubeAttachments(count);
		for (size_t i = 0; i < CubeAttachments.size(); i++)
			CubeAttachments[i].Create(format, extent, hasMipmap, sampleCount, otherUsages);

		const int id = m_color_attachment_id++;

		auto [it1, ok1] = mCubeAttachments.emplace(id, std::move(CubeAttachments));
		if (!ok1) {
			std::cerr << std::format("[ERROR] ImageManager: Emplace CubeAttachments for '{}' (id={}) failed\n", name, id);
			return { M_INVALID_ID, {} };
		}

		auto [it2, ok2] = mCubeAttachmentIDs.emplace(name, id);
		if (!ok2) {
			std::cerr << std::format("[WARNING] ImageManager: CubeAttachment '{}' has not been recorded!\n", name);
			mCubeAttachments.erase(id);
			return { M_INVALID_ID, {} };
		}

		auto& vec = it1->second;
		return { id, std::span<CubeAttachment>(vec.data(), vec.size()) };
	}

	std::pair<int, std::span<CubeAttachment>> ImageManager::GetCubeAttachments(std::string name)
	{
		if (auto it = mCubeAttachmentIDs.find(name); it != mCubeAttachmentIDs.end())
			return GetCubeAttachments(it->second);
		std::cerr << std::format("[ERROR] ImageManager: CubeAttachments with name '{}' do not exist!\n", name);
		return { M_INVALID_ID, {} };
	}

	std::pair<int, std::span<CubeAttachment>> ImageManager::GetCubeAttachments(int id)
	{
		if (auto it = mCubeAttachments.find(id); it != mCubeAttachments.end())
			return { id, std::span<CubeAttachment>(it->second.data(), it->second.size()) };
		std::cerr << std::format("[ERROR] ImageManager: CubeAttachments with id {} do not exist!\n", id);
		return { M_INVALID_ID, {} };
	}

	int ImageManager::DestroyCubeAttachments(std::string name)
	{
		if (auto it = mCubeAttachmentIDs.find(name); it != mCubeAttachmentIDs.end())
			return DestroyCubeAttachments(it->second);
		std::cerr << std::format("[WARNING] ImageManager: CubeAttachments with name '{}' do not exist!\n", name);
		return M_INVALID_ID;
	}

	int ImageManager::DestroyCubeAttachments(int id)
	{
		if (auto it = mCubeAttachments.find(id); it != mCubeAttachments.end()) {
			it->second.clear();
			return it->first;
		}
		std::cerr << std::format("[WARNING] ImageManager: CubeAttachments with id {} do not exist!\n", id);
		return M_INVALID_ID;
	}

	bool ImageManager::HasCubeAttachments(std::string name)
	{
		if (auto it = mCubeAttachmentIDs.find(name); it != mCubeAttachmentIDs.end())
			return HasCubeAttachments(it->second);
		return false;
	}

	bool ImageManager::HasCubeAttachments(int id)
	{
		return mCubeAttachments.contains(id);
	}

	size_t ImageManager::GetCubeAttachmentsCount() const
	{
		return mCubeAttachments.size();;
	}


}