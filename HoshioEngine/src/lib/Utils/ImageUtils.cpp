#include "Utils/ImageUtils.h"

namespace HoshioEngine {


	uint32_t ImageUtils::CalculateMipLevelCount(VkExtent2D extent)
	{
		return uint32_t(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;
	}

	const VkFormatProperties ImageUtils::FormatProperties(VkFormat format)
	{
		return VulkanPlus::Plus().FormatProperties(format);
	}

	void ImageUtils::CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, const VkBufferImageCopy& region, ImageBarrierInfo imgBarrier_from, ImageBarrierInfo imgBarrier_to)
	{
		VkImageMemoryBarrier imageMemoryBarrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = imgBarrier_from.access,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.oldLayout = imgBarrier_from.layout,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = dstImage,
			.subresourceRange = {
				.aspectMask = region.imageSubresource.aspectMask,
				.baseMipLevel = region.imageSubresource.mipLevel,
				.levelCount = 1,
				.baseArrayLayer = region.imageSubresource.baseArrayLayer,
				.layerCount = region.imageSubresource.layerCount
			}
		};

		if (imgBarrier_from.isNeeded) {
			vkCmdPipelineBarrier(commandBuffer,
				imgBarrier_from.stage, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
		}

		vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		if (imgBarrier_to.isNeeded) {
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = imgBarrier_to.access;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = imgBarrier_to.layout;
			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, imgBarrier_to.stage,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
		}
	}

	void ImageUtils::CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage, const VkImageBlit& region, ImageBarrierInfo dstImgBarrier_from, ImageBarrierInfo dstImgBarrier_to, VkFilter filter)
	{
		VkImageMemoryBarrier imageMemoryBarrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = dstImgBarrier_from.access,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.oldLayout = dstImgBarrier_from.layout,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = dstImage,
			.subresourceRange = {
				.aspectMask = region.dstSubresource.aspectMask,
				.baseMipLevel = region.dstSubresource.mipLevel,
				.levelCount = 1,
				.baseArrayLayer = region.dstSubresource.baseArrayLayer,
				.layerCount = region.dstSubresource.layerCount
			}
		};

		if (dstImgBarrier_from.isNeeded) {
			vkCmdPipelineBarrier(commandBuffer,
				dstImgBarrier_from.stage, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
		}

		vkCmdBlitImage(commandBuffer, 
			srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &region, filter);

		if (dstImgBarrier_to.isNeeded) {
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = dstImgBarrier_to.access;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemoryBarrier.newLayout = dstImgBarrier_to.layout;
			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, dstImgBarrier_to.stage,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
		}
	}

	void ImageUtils::CmdImagePipelineBarrier(VkCommandBuffer commandBuffer, VkImage image, const VkImageSubresourceRange& subresourceRange, ImageBarrierInfo srcImgBarrier, ImageBarrierInfo dstImgBarrier)
	{
		VkImageMemoryBarrier imageMemoryBarrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = srcImgBarrier.access,
			.dstAccessMask = dstImgBarrier.access,
			.oldLayout = srcImgBarrier.layout,
			.newLayout = dstImgBarrier.layout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = subresourceRange
		};

		vkCmdPipelineBarrier(commandBuffer,
			srcImgBarrier.stage, dstImgBarrier.stage,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}

	void ImageUtils::CmdGenerateMipmap2D(VkCommandBuffer commandBuffer, VkImage image, VkExtent2D imageExtent, uint32_t mipLevelCount, uint32_t layerCount, ImageBarrierInfo imgBarrier_to, VkFilter minFilter)
	{
		auto MipmapExtent = [](VkExtent2D imageExtent, uint32_t mipLevel) {
			VkOffset3D extent = { std::max(int32_t(imageExtent.width >> mipLevel), 1), std::max(int32_t(imageExtent.height >> mipLevel), 1), 1 };
			return extent;
		};

		for (uint32_t i = 1; i < mipLevelCount; i++) {
			VkImageBlit region = {
				{ VK_IMAGE_ASPECT_COLOR_BIT, i - 1, 0, layerCount },
				{ {}, MipmapExtent(imageExtent, i - 1) },           
				{ VK_IMAGE_ASPECT_COLOR_BIT, i, 0, layerCount },    
				{ {}, MipmapExtent(imageExtent, i) }
			};

			CmdBlitImage(commandBuffer, image, image, region,
				ImageBarrierInfo{ VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_IMAGE_LAYOUT_UNDEFINED },
				ImageBarrierInfo{ VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL },
				minFilter);
		}

		if (imgBarrier_to.isNeeded)
			CmdImagePipelineBarrier(commandBuffer, image,
				VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevelCount, 0, layerCount },
				ImageBarrierInfo{ VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL },
				imgBarrier_to);
	}

}