#ifndef _IMAGE_MANAGER_H_
#define _IMAGE_MANAGER_H_

#include "Plus/BufferManager.h"
#include "Base/RpwfManager.h"

namespace HoshioEngine {
	class Texture {
	protected:
		ImageMemory imageMemory;
		ImageView imageView;
		Texture() = default;
		void CreateImageMemory(VkImageType imageType, VkFormat format, VkExtent3D extent, uint32_t mipLevelCount, uint32_t arrayLayerCount, VkImageCreateFlags flags = 0);
		void CreateImageView(VkImageViewType viewType, VkFormat format, uint32_t mipLevelCount, uint32_t arrayLayerCount, VkImageViewCreateFlags flags = 0);
		//static std::unique_ptr<uint8_t[]> LoadFile_Internal(const auto* address, size_t fileSize, VkExtent2D& extent, VkFormat format);
		static std::unique_ptr<uint8_t[]> LoadFile_Internal(const char* address, size_t fileSize, VkExtent2D& extent, VkFormat format);
		static std::unique_ptr<uint8_t[]> LoadFile_Internal(const uint8_t* address, size_t fileSize, VkExtent2D& extent, VkFormat format);
	public:
		VkImageView ImageView() const;
		VkImage Image() const;
		const VkImageView* AddressOfImageView() const;
		const VkImage* AddressOfImage() const;

		VkDescriptorImageInfo DescriptorImageInfo(VkSampler sampler) const;

		[[nodiscard]]
		static std::unique_ptr<uint8_t[]> LoadFile(const char* filePath, VkExtent2D& extent, VkFormat format);

		[[nodiscard]]
		static std::unique_ptr<uint8_t[]> LoadFile(const uint8_t* fileBinaries, size_t fileSize, VkExtent2D& extent, VkFormat format);

		static void CopyBlitAndGenerateMipmap2D(VkBuffer buffer_copyFrom, VkImage image_copyTo, VkImage image_blitTo, VkExtent2D imageExtent,
			uint32_t mipLevelCount = 1, uint32_t layerCount = 1, VkFilter minFilter = VK_FILTER_LINEAR);

		static void BlitAndGenerateMipmap2D(VkImage image_preinitialized, VkImage image_final, VkExtent2D imageExtent,
			uint32_t mipLevelCount = 1, uint32_t layerCount = 1, VkFilter minFilter = VK_FILTER_LINEAR);
	};

	class Texture2D : public Texture {
	protected:
		VkExtent2D extent = {};
		uint32_t mipLevelCount = 1;
		std::vector<HoshioEngine::ImageView> imageViews;
		void Create_Internal(VkFormat initial_format, VkFormat final_format, bool generateMip = true);

	public:
		Texture2D() = default;
		Texture2D(const char* filePath, VkFormat initial_format, VkFormat final_format, bool generateMip = true);
		Texture2D(const uint8_t* pImageData, VkExtent2D extent, VkFormat initial_format, VkFormat final_format, bool generateMip = true);

		Texture::DescriptorImageInfo;
		VkExtent2D Extent() const;
		uint32_t Width() const;
		uint32_t Height() const;
		uint32_t MipLevelCount() const;
		VkDescriptorImageInfo DescriptorImageInfo(VkSampler sampler, uint32_t mipLevel) const;

		void Create(const char* filePath, VkFormat initial_format, VkFormat final_format, bool generateMip = true);
		void Create(const uint8_t* pImageData, VkExtent2D extent, VkFormat initial_format, VkFormat final_format, bool generateMip = true);
	};

	class Attachment {
	protected:
		ImageMemory imageMemory;
		ImageView imageView;
		Attachment() = default;
	public:
		VkImageView ImageView() const;
		VkImage Image() const;
		const VkImageView* AddressOfImageView() const;
		const VkImage* AddressOfImage() const;

		VkDescriptorImageInfo DescriptorImageInfo(VkSampler sampler) const;
	};

	class ColorAttachment : public Attachment {
	private:
		std::vector<HoshioEngine::ImageView> imageViews;
		uint32_t mipLevelCount = 1;
	public:
		Attachment::ImageView;
		Attachment::AddressOfImageView;
		Attachment::DescriptorImageInfo;
		VkImageView ImageView(uint32_t mipLevel) const;
		const VkImageView* AddressOfImageView(uint32_t mipLevel) const;
		uint32_t MipLevelCount() const;
		VkDescriptorImageInfo DescriptorImageInfo(VkSampler sampler, uint32_t mipLevel) const;
		ColorAttachment() = default;
		ColorAttachment(VkFormat format, VkExtent2D extent, bool hasMipmap = true, uint32_t layerCount = 1,
			VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags otherUsages = 0);

		void Create(VkFormat format, VkExtent2D extent, bool hasMipmap = true, uint32_t layerCount = 1,
			VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags otherUsages = 0);

		static bool FormatIsSupported(VkFormat format, bool supportBlending = true);
	};

	class DepthStencilAttachment : public Attachment{
	public:
		DepthStencilAttachment() = default;
		DepthStencilAttachment(VkFormat format, VkExtent2D extent, bool stencilOnly = false, uint32_t layerCount = 1,
			VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags otherUsages = 0);

		void Create(VkFormat format, VkExtent2D extent, bool stencilOnly = false, uint32_t layerCount = 1,
			VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags otherUsages = 0);

		static bool FormatIsSupported(VkFormat format);
	};

	class ImageManager {
	private:
		int m_texture2d_id = 0;
		int m_color_attachment_id = 0;
		int m_depth_stencil_attachment_id = 0;

		std::unordered_map<std::string, int> mTexture2DIDs;
		std::unordered_map<int, Texture2D> mTexture2Ds;
		std::unordered_map<std::string, int> mColorAttachmentIDs;
		std::unordered_map<int, std::vector<ColorAttachment>> mColorAttachments;
		std::unordered_map<std::string, int> mDepthStencilAttachmentIDs;
		std::unordered_map<int, std::vector<DepthStencilAttachment>> mDepthStencilAttachments;

		std::pair<int, std::span<Texture2D>> RecreateTexture2D(int id, const char* filePath, VkFormat initial_format, VkFormat final_format, bool generateMip = true);
		std::pair<int, std::span<Texture2D>> RecreateTexture2D(int id, const uint8_t* pImageData, VkExtent2D extent, VkFormat initial_format, VkFormat final_format, bool generateMip = true);
		std::pair<int, std::span<ColorAttachment>> RecreateColorAttachments(int id, uint32_t count, VkFormat format, VkExtent2D extent, bool hasMipmap = true, uint32_t layerCount = 1,
			VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags otherUsages = 0);
		std::pair<int, std::span<DepthStencilAttachment>> RecreateDepthStencilAttachments(int id, uint32_t count, VkFormat format, VkExtent2D extent, bool stencilOnly = false, uint32_t layerCount = 1,
			VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags otherUsages = 0);
	public:

		std::pair<int, std::span<Texture2D>> CreateTexture2D(std::string name, const char* filePath, VkFormat initial_format, VkFormat final_format, bool generateMip = true);
		std::pair<int, std::span<Texture2D>> CreateTexture2D(std::string name, const uint8_t* pImageData, VkExtent2D extent, VkFormat initial_format, VkFormat final_format, bool generateMip = true);
		std::pair<int, std::span<Texture2D>> GetTexture2D(std::string name);
		std::pair<int, std::span<Texture2D>> GetTexture2D(int id);
		bool HasTexture2D(std::string name);
		bool HasTexture2D(int id);
		size_t GetTexture2DCount() const;


		std::pair<int, std::span<ColorAttachment>> CreateColorAttachments(std::string name, uint32_t count, VkFormat format, VkExtent2D extent, bool hasMipmap = true, uint32_t layerCount = 1,
			VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags otherUsages = 0);
		std::pair<int, std::span<ColorAttachment>> GetColorAttachments(std::string name);
		std::pair<int, std::span<ColorAttachment>> GetColorAttachments(int id);
		int DestroyColorAttachments(std::string name);
		int DestroyColorAttachments(int id);
		bool HasColorAttachments(std::string name);
		bool HasColorAttachments(int id);
		size_t GetColorAttachmentsCount() const;

		std::pair<int, std::span<DepthStencilAttachment>> CreateDepthStencilAttachments(std::string name, uint32_t count, VkFormat format, VkExtent2D extent, bool stencilOnly = false, uint32_t layerCount = 1,
			VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags otherUsages = 0);
		std::pair<int, std::span<DepthStencilAttachment>> GetDepthStencilAttachments(std::string name);
		std::pair<int, std::span<DepthStencilAttachment>> GetDepthStencilAttachments(int id);
		int DestroyDepthStencilAttachments(std::string name);
		int DestroyDepthStencilAttachments(int id);
		bool HasDepthStencilAttachments(std::string name);
		bool HasDepthStencilAttachments(int id);
		size_t GetDepthStencilAttachmentsCount() const;


	};
}

#endif // !_IMAGE_MANAGER_H_
