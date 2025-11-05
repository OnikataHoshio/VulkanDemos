#ifndef _IMAGE_UTILS_H_
#define _IMAGE_UTILS_H_

#include "Plus/VulkanPlus.h"

namespace HoshioEngine {

	struct ImageBarrierInfo {
		const bool isNeeded = false;
		const VkPipelineStageFlags stage = 0;
		const VkAccessFlags access = 0;
		const VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

		constexpr ImageBarrierInfo() = default;
		constexpr ImageBarrierInfo(VkPipelineStageFlags stage, VkAccessFlags access, VkImageLayout layout)
			:isNeeded(true), stage(stage), access(access), layout(layout) {};
	};

	struct ImageUtils {

		static uint32_t CalculateMipLevelCount(VkExtent2D extent);

		static const VkFormatProperties FormatProperties(VkFormat format);

		static void CmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage, 
			const VkBufferImageCopy& region, ImageBarrierInfo imgBarrier_from, ImageBarrierInfo imgBarrier_to);

		static void CmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImage dstImage, 
			const VkImageBlit& region, ImageBarrierInfo dstImgBarrier_from, ImageBarrierInfo dstImgBarrier_to, VkFilter filter = VK_FILTER_LINEAR);

		static void CmdImagePipelineBarrier(VkCommandBuffer commandBuffer, VkImage image,
			const VkImageSubresourceRange& subresourceRange, ImageBarrierInfo srcImgBarrier, ImageBarrierInfo dstImgBarrier);

		static void CmdGenerateMipmap2D(VkCommandBuffer commandBuffer, VkImage image, VkExtent2D imageExtent, uint32_t mipLevelCount, uint32_t layerCount,
			ImageBarrierInfo  imgBarrier_to, VkFilter minFilter = VK_FILTER_LINEAR);
	};
}


#endif // !_IMAGE_UTILS_H_

