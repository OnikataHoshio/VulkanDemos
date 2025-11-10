#include "Plus/VulkanPlus.h"
#include "Base/SyncManager.h"

namespace HoshioEngine {
	VulkanPlus::VulkanPlus()
	{
		if (VulkanBase::Base().QueueFamilyIndex_Graphics() != VK_QUEUE_FAMILY_IGNORED) {
			commandPool_graphics.Create(VulkanBase::Base().QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
			commandPool_graphics.Allocate(commandBuffer_graphics);
			commandPool_graphics.Allocate(commandBuffer_present);
		}
		if (VulkanBase::Base().QueueFamilyIndex_Compute() != VK_QUEUE_FAMILY_IGNORED) {
			commandPool_compute.Create(VulkanBase::Base().QueueFamilyIndex_Compute(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		}

		for (uint32_t i = 0; i < FORMAT_COUNT; i++)
			vkGetPhysicalDeviceFormatProperties(VulkanBase::Base().PhysicalDevice(), VkFormat(i), &formatProperties[i]);

		VkDescriptorPoolSize pool_sizes[11] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		descriptorPool.Create(1000 * XS_ARRAYSIZE(pool_sizes), pool_sizes, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

		DefaultVertex defaultQuadVertex[4] = {
		{ { -1.f, -1.f }, { 0, 0 } },
		{ {  1.f, -1.f }, { 1, 0 } },
		{ { -1.f,  1.f }, { 0, 1 } },
		{ {  1.f,  1.f }, { 1, 1 } }
		};
		defaultVertexBuffer.Create(sizeof defaultQuadVertex)
			.TransferData(defaultQuadVertex);

		InitSwapchainFramebuffers();

	}

	void VulkanPlus::InitSwapchainRenderPass() {

		{
			VkAttachmentDescription attachmentDescription = {
			.format = VulkanBase::Base().SwapchainCi().imageFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			};

			VkAttachmentReference attachmentReference = {
				.attachment = 0,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			};


			VkSubpassDescription subpassDescription = {
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments = &attachmentReference,
			};
			VkSubpassDependency subpassDependency = {
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_SHADER_READ_BIT
			};

			VkRenderPassCreateInfo renderPassCreateInfo = {
				.attachmentCount = 1,
				.pAttachments = &attachmentDescription,
				.subpassCount = 1,
				.pSubpasses = &subpassDescription,
				.dependencyCount = 1,
				.pDependencies = &subpassDependency
			};
			swapchainRenderPass.Create(renderPassCreateInfo);
		}
		
		{
			VkAttachmentDescription attachmentDescriptions[2] = {
				{
					.format = VulkanBase::Base().SwapchainCi().imageFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				},
				{
					.format = VK_FORMAT_D24_UNORM_S8_UINT,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				}
			};

			VkAttachmentReference attachmentReferences[2] = {
				{
					.attachment = 0,
					.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				},
				{
					.attachment = 1,
					.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				}
			};

			VkSubpassDescription subpassDescription = {
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments = attachmentReferences,
				.pDepthStencilAttachment = attachmentReferences + 1
			};

			VkSubpassDependency subpassDependency = {
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT
			};

			VkRenderPassCreateInfo renderPassCreateInfo = {
				.attachmentCount = 2,
				.pAttachments = attachmentDescriptions,
				.subpassCount = 1,
				.pSubpasses = &subpassDescription,
				.dependencyCount = 1,
				.pDependencies = &subpassDependency
			};
			
			swapchainRenderPassWithDepthStencil.Create(renderPassCreateInfo);
		}
	}

	void VulkanPlus::InitSwapchainFramebuffers()
	{
		InitSwapchainRenderPass();
		auto Create = [&] {
			{
				swapchainFramebuffers.resize(VulkanBase::Base().SwapchainImageCount());
				VkFramebufferCreateInfo framebufferCreateInfo = {
					.renderPass = swapchainRenderPass,
					.attachmentCount = 1,
					.width = VulkanBase::Base().SwapchainCi().imageExtent.width,
					.height = VulkanBase::Base().SwapchainCi().imageExtent.height,
					.layers = 1
				};
				for (uint32_t i = 0; i < VulkanBase::Base().SwapchainImageCount(); i++) {
					VkImageView attachment = VulkanBase::Base().SwapchainImageView(i);
					framebufferCreateInfo.pAttachments = &attachment;
					swapchainFramebuffers[i].Create(framebufferCreateInfo);
				}
			}
			
			{
				swapchainFramebuffersWithDepthStencil.resize(VulkanBase::Base().SwapchainImageCount());
				swapchainDepthStencilAttachments.resize(VulkanBase::Base().SwapchainImageCount());
				for (auto& dsAttachment : swapchainDepthStencilAttachments)
					dsAttachment.Create(VK_FORMAT_D24_UNORM_S8_UINT, VulkanBase::Base().SwapchainCi().imageExtent,
						false, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
				VkFramebufferCreateInfo framebufferCreateInfo = {
					.renderPass = swapchainRenderPassWithDepthStencil,
					.attachmentCount = 2,
					.width = VulkanBase::Base().SwapchainExtent().width,
					.height = VulkanBase::Base().SwapchainExtent().height,
					.layers = 1
				};
				std::vector<VkImageView> attachments(2);
				for (uint32_t i = 0; i < VulkanBase::Base().SwapchainImageCount(); i++) {
					attachments[0] = VulkanBase::Base().SwapchainImageView(i);
					attachments[1] = swapchainDepthStencilAttachments[i].ImageView();
					framebufferCreateInfo.pAttachments = attachments.data();
					swapchainFramebuffersWithDepthStencil[i].Create(framebufferCreateInfo);
				}
			}


			};

		auto Destroy = [&] {
			if (!swapchainFramebuffers.empty())
				swapchainFramebuffers.clear();
			if (!swapchainFramebuffersWithDepthStencil.empty())
				swapchainFramebuffersWithDepthStencil.clear();
			if (!swapchainDepthStencilAttachments.empty())
				swapchainDepthStencilAttachments.clear();
			};

		Create();
		VulkanBase::Base().AddCallback_CreateSwapchain(Create);
		VulkanBase::Base().AddCallback_DestroySwapchain(Destroy);
	}

	VulkanPlus& VulkanPlus::Plus()
	{
		static VulkanPlus vulkanPlus;
		return vulkanPlus;
	}
	const VkFormatProperties& VulkanPlus::FormatProperties(VkFormat format) const
	{
		if (uint32_t(format) >= FORMAT_COUNT)
			throw std::runtime_error("This function only supports definite formats less than 184.");
		return formatProperties[uint32_t(format)];
	}
	const CommandPool& VulkanPlus::CommandPool_Graphics() const
	{
		return commandPool_graphics;
	}
	const CommandPool& VulkanPlus::CommandPool_Compute() const
	{
		return commandPool_compute;
	}
	const CommandBuffer& VulkanPlus::CommandBuffer_Graphics() const
	{
		return commandBuffer_graphics;
	}
	const CommandBuffer& VulkanPlus::CommandBuffer_Present() const
	{
		return commandBuffer_present;
	}
	const CommandBuffer& VulkanPlus::CommandBuffer_Transfer() const
	{
		return commandBuffer_graphics;
	}
	const DescriptorPool& VulkanPlus::DescriptorPool() const
	{
		return descriptorPool;
	}
	const VertexBuffer& VulkanPlus::DefaultVertexBuffer() const
	{
		return defaultVertexBuffer;
	}
	const Framebuffer& VulkanPlus::CurrentSwapchainFramebuffer() const
	{
		return swapchainFramebuffers[VulkanBase::Base().CurrentImageIndex()];
	}
	const Framebuffer& VulkanPlus::CurrentSwapchainFramebufferWithDepthStencil() const
	{
		return swapchainFramebuffersWithDepthStencil[VulkanBase::Base().CurrentImageIndex()];
	}
	const std::vector<Framebuffer>& VulkanPlus::SwapchainFramebuffers() const
	{
		return swapchainFramebuffers;
	}
	const std::vector<Framebuffer>& VulkanPlus::SwapchainFramebuffersWithDepthStencil() const
	{
		return swapchainFramebuffersWithDepthStencil;
	}
	const RenderPass& VulkanPlus::SwapchainRenderPass() const
	{
		return swapchainRenderPass;
	}

	const RenderPass& VulkanPlus::SwapchainRenderPassWithDepthStencil() const
	{
		return swapchainRenderPassWithDepthStencil;
	}

	void VulkanPlus::ExecuteCommandBuffer_Graphics(VkCommandBuffer commandBuffer) const
	{
		Fence fence;
		VkSubmitInfo submitInfo = {
			.commandBufferCount = 1,
			.pCommandBuffers = &commandBuffer
		};
		VulkanBase::Base().SubmitCommandBuffer_Graphics(submitInfo, fence);
		fence.Wait();
	}

	std::pair<int, std::span<Texture2D>> VulkanPlus::CreateTexture2D(std::string name, const char* filePath, VkFormat initial_format, VkFormat final_format, bool generateMip)
	{
		return image_manager.CreateTexture2D(std::move(name), filePath, initial_format, final_format, generateMip);
	}

	std::pair<int, std::span<Texture2D>> VulkanPlus::CreateTexture2D(std::string name, const uint8_t* pImageData, VkExtent2D extent, VkFormat initial_format, VkFormat final_format, bool generateMip)
	{
		return image_manager.CreateTexture2D(std::move(name), pImageData, extent, initial_format, final_format, generateMip);
	}

	std::pair<int, std::span<Texture2D>> VulkanPlus::GetTexture2D(std::string name)
	{
		return image_manager.GetTexture2D(std::move(name));
	}

	std::pair<int, std::span<Texture2D>> VulkanPlus::GetTexture2D(int id)
	{
		return image_manager.GetTexture2D(id);
	}

	bool VulkanPlus::HasTexture2D(std::string name)
	{
		return image_manager.HasTexture2D(std::move(name));
	}

	bool VulkanPlus::HasTexture2D(int id)
	{
		return image_manager.HasTexture2D(id);
	}

	size_t VulkanPlus::GetTextureCount() const
	{
		return image_manager.GetTexture2DCount();
	}

	std::pair<int, std::span<TextureArray>> VulkanPlus::CreateTextureArray(std::string name, const char* filepath, VkExtent2D extentInTiles, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		return image_manager.CreateTextureArray(std::move(name), filepath, extentInTiles, format_initial, format_final, generateMipmap);
	}

	std::pair<int, std::span<TextureArray>> VulkanPlus::CreateTextureArray(std::string name, const uint8_t* pImageData, VkExtent2D fullExtent, VkExtent2D extentInTiles, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		return image_manager.CreateTextureArray(std::move(name), pImageData, fullExtent, extentInTiles, format_initial, format_final, generateMipmap);
	}

	std::pair<int, std::span<TextureArray>> VulkanPlus::CreateTextureArray(std::string name, ArrayRef<const char* const> filepaths, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		return image_manager.CreateTextureArray(std::move(name), filepaths, format_initial, format_final, generateMipmap);
	}

	std::pair<int, std::span<TextureArray>> VulkanPlus::CreateTextureArray(std::string name, ArrayRef<const uint8_t* const> psImageData, VkExtent2D extent, VkFormat format_initial, VkFormat format_final, bool generateMipmap)
	{
		return image_manager.CreateTextureArray(std::move(name), psImageData, extent, format_initial, format_final, generateMipmap);
	}

	std::pair<int, std::span<TextureArray>> VulkanPlus::GetTextureArray(std::string name)
	{
		return image_manager.GetTextureArray(std::move(name));
	}

	std::pair<int, std::span<TextureArray>> VulkanPlus::GetTextureArray(int id)
	{
		return image_manager.GetTextureArray(id);
	}

	bool VulkanPlus::HasTextureArray(std::string name)
	{
		return image_manager.HasTextureArray(std::move(name));
	}

	bool VulkanPlus::HasTextureArray(int id)
	{
		return image_manager.HasTextureArray(id);
	}

	size_t VulkanPlus::GetTextureArrayCount() const
	{
		return image_manager.GetTextureArrayCount();
	}

	std::pair<int, std::span<TextureCube>> VulkanPlus::CreateTextureCube(std::string name, const char* filepath, const glm::uvec2 facePositions[6], VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap)
	{
		return image_manager.CreateTextureCube(std::move(name), filepath, facePositions, format_initial, format_final, lookFromOutside, generateMipmap);
	}

	std::pair<int, std::span<TextureCube>> VulkanPlus::CreateTextureCube(std::string name, const uint8_t* pImageData, VkExtent2D fullExtent, const glm::uvec2 facePositions[6], VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap)
	{
		return image_manager.CreateTextureCube(std::move(name), pImageData, fullExtent, facePositions, format_initial, format_final, lookFromOutside, generateMipmap);
	}

	std::pair<int, std::span<TextureCube>> VulkanPlus::CreateTextureCube(std::string name, const char* const* filepaths, VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap)
	{
		return image_manager.CreateTextureCube(std::move(name), filepaths, format_initial, format_final, lookFromOutside, generateMipmap);
	}

	std::pair<int, std::span<TextureCube>> VulkanPlus::CreateTextureCube(std::string name, const uint8_t* const* psImageData, VkExtent2D extent, VkFormat format_initial, VkFormat format_final, bool lookFromOutside, bool generateMipmap)
	{
		return image_manager.CreateTextureCube(std::move(name), psImageData, extent, format_initial, format_final, lookFromOutside, generateMipmap);
	}

	std::pair<int, std::span<TextureCube>> VulkanPlus::GetTextureCube(std::string name)
	{
		return image_manager.GetTextureCube(std::move(name));
	}

	std::pair<int, std::span<TextureCube>> VulkanPlus::GetTextureCube(int id)
	{
		return image_manager.GetTextureCube(id);
	}

	bool VulkanPlus::HasTextureCube(std::string name)
	{
		return image_manager.HasTextureCube(std::move(name));
	}

	bool VulkanPlus::HasTextureCube(int id)
	{
		return image_manager.HasTextureCube(id);
	}

	size_t VulkanPlus::GetTextureCubeCount() const
	{
		return image_manager.GetTextureCubeCount();
	}

	std::pair<int, std::span<ColorAttachment>> VulkanPlus::CreateColorAttachments(std::string name, uint32_t count, VkFormat format, VkExtent2D extent, bool hasMipmap, uint32_t layerCount, VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages)
	{
		return image_manager.CreateColorAttachments(std::move(name), count, format, extent, hasMipmap, layerCount, sampleCount, otherUsages);
	}

	int VulkanPlus::DestroyColorAttachments(std::string name)
	{
		return image_manager.DestroyColorAttachments(std::move(name));
	}

	int VulkanPlus::DestroyColorAttachments(int id)
	{
		return image_manager.DestroyColorAttachments(id);
	}

	std::pair<int, std::span<ColorAttachment>> VulkanPlus::GetColorAttachments(std::string name)
	{
		return image_manager.GetColorAttachments(std::move(name));
	}

	std::pair<int, std::span<ColorAttachment>> VulkanPlus::GetColorAttachments(int id)
	{
		return image_manager.GetColorAttachments(id);
	}

	bool VulkanPlus::HasColorAttachments(std::string name)
	{
		return image_manager.HasColorAttachments(std::move(name));
	}

	bool VulkanPlus::HasColorAttachments(int id)
	{
		return image_manager.HasColorAttachments(id);
	}

	size_t VulkanPlus::GetColorAttachmentsCount() const
	{
		return image_manager.GetColorAttachmentsCount();
	}

	std::pair<int, std::span<DepthStencilAttachment>> VulkanPlus::CreateDepthStencilAttachments(std::string name, uint32_t count, VkFormat format, VkExtent2D extent, bool stencilOnly, uint32_t layerCount, VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages)
	{
		return image_manager.CreateDepthStencilAttachments(std::move(name), count, format, extent, stencilOnly, layerCount, sampleCount, otherUsages);
	}
	int VulkanPlus::DestroyDepthStencilAttachments(std::string name)
	{
		return image_manager.DestroyDepthStencilAttachments(std::move(name));
	}
	int VulkanPlus::DestroyDepthStencilAttachments(int id)
	{
		return image_manager.DestroyDepthStencilAttachments(id);
	}
	std::pair<int, std::span<DepthStencilAttachment>> VulkanPlus::GetDepthStencilAttachments(std::string name)
	{
		return image_manager.GetDepthStencilAttachments(std::move(name));
	}
	std::pair<int, std::span<DepthStencilAttachment>> VulkanPlus::GetDepthStencilAttachments(int id)
	{
		return image_manager.GetDepthStencilAttachments(id);
	}

	bool VulkanPlus::HasDepthStencilAttachments(std::string name)
	{
		return image_manager.HasDepthStencilAttachments(std::move(name));
	}

	bool VulkanPlus::HasDepthStencilAttachments(int id)
	{
		return image_manager.HasDepthStencilAttachments(id);
	}

	size_t VulkanPlus::GetDepthStencilAttachmentsCount() const
	{
		return image_manager.GetDepthStencilAttachmentsCount();
	}

	std::pair<int, std::span<CubeAttachment>> VulkanPlus::CreateCubeAttachments(std::string name, uint32_t count, VkFormat format, VkExtent2D extent, bool hasMipmap, VkSampleCountFlagBits sampleCount, VkImageUsageFlags otherUsages)
	{
		return image_manager.CreateCubeAttachments(std::move(name), count, format, extent, hasMipmap, sampleCount, otherUsages);
	}

	std::pair<int, std::span<CubeAttachment>> VulkanPlus::GetCubeAttachments(std::string name)
	{
		return image_manager.GetCubeAttachments(std::move(name));
	}

	std::pair<int, std::span<CubeAttachment>> VulkanPlus::GetCubeAttachments(int id)
	{
		return image_manager.GetCubeAttachments(id);
	}

	int VulkanPlus::DestroyCubeAttachments(std::string name)
	{
		return image_manager.DestroyCubeAttachments(std::move(name));
	}

	int VulkanPlus::DestroyCubeAttachments(int id)
	{
		return image_manager.DestroyCubeAttachments(id);
	}

	bool VulkanPlus::HasCubeAttachments(std::string name)
	{
		return image_manager.HasCubeAttachments(std::move(name));
	}

	bool VulkanPlus::HasCubeAttachments(int id)
	{
		return image_manager.HasCubeAttachments(id);
	}

	size_t VulkanPlus::GetCubeAttachmentsCount() const
	{
		return image_manager.GetCubeAttachmentsCount();
	}

	std::pair<int, std::span<Framebuffer>> VulkanPlus::CreateFramebuffers(std::string name, uint32_t count, std::vector<VkFramebufferCreateInfo>& createInfos) {
		return rpwf_manager.CreateFramebuffers(std::move(name), count, createInfos);
	}
	std::pair<int, std::span<Framebuffer>> VulkanPlus::GetFramebuffers(std::string name) {
		return rpwf_manager.GetFramebuffers(std::move(name));
	}
	std::pair<int, std::span<Framebuffer>> VulkanPlus::GetFramebuffers(int id) {
		return rpwf_manager.GetFramebuffers(id);
	}
	int VulkanPlus::DestroyFramebuffers(std::string name) {
		return rpwf_manager.DestroyFramebuffers(name);
	}
	int VulkanPlus::DestroyFramebuffers(int id) {
		return rpwf_manager.DestroyFramebuffers(id);
	}
	bool VulkanPlus::HasFramebuffers(std::string name) {
		return rpwf_manager.HasFramebuffer(name);
	}
	bool VulkanPlus::HasFramebuffers(int id) {
		return rpwf_manager.HasFramebuffer(id);
	}
	size_t VulkanPlus::GetFramebuffersCount() const {
		return rpwf_manager.GetFramebuffersCount();
	}
	
	std::pair<int, std::span<Fence>> VulkanPlus::CreateFences(std::string name, uint32_t count, VkFenceCreateFlags flags)
	{
		return sync_manager.CreateFences(std::move(name), count, flags);
	}
	std::pair<int, std::span<Fence>> VulkanPlus::GetFences(std::string name)
	{
		return sync_manager.GetFences(std::move(name));
	}
	std::pair<int, std::span<Fence>> VulkanPlus::GetFences(int id)
	{
		return sync_manager.GetFences(id);
	}

	bool VulkanPlus::HasFence(std::string name)
	{
		return sync_manager.HasFence(std::move(name));
	}

	bool VulkanPlus::HasFence(int id)
	{
		return sync_manager.HasFence(id);
	}

	size_t VulkanPlus::GetFencesCount() const
	{
		return sync_manager.GetFencesCount();
	}

	std::pair<int, std::span<Semaphore>> VulkanPlus::CreateSemaphores(std::string name, uint32_t count, VkSemaphoreCreateFlags flags)
	{
		return sync_manager.CreateSemaphores(std::move(name), count, flags);
	}
	std::pair<int, std::span<Semaphore>> VulkanPlus::GetSemaphores(std::string name)
	{
		return sync_manager.GetSemaphores(std::move(name));
	}
	std::pair<int, std::span<Semaphore>> VulkanPlus::GetSemaphores(int id)
	{
		return sync_manager.GetSemaphores(id);
	}
	bool VulkanPlus::HasSemaphore(std::string name)
	{
		return sync_manager.HasSemaphore(std::move(name));
	}
	bool VulkanPlus::HasSemaphore(int id)
	{
		return sync_manager.HasSemaphore(id);
	}
	size_t VulkanPlus::GetSemaphoresCount() const
	{
		return sync_manager.GetSemaphoresCount();
	}

	std::pair<int, std::span<ShaderModule>> VulkanPlus::CreateShaderModule(std::string name, const char* filePath)
	{
		return pipeline_manager.CreateShaderModule(std::move(name), filePath);
	}

	std::pair<int, std::span<ShaderModule>> VulkanPlus::CreateShaderModule(std::string name, size_t codeSize, const uint32_t* pCode)
	{
		return pipeline_manager.CreateShaderModule(std::move(name), codeSize, pCode);
	}
	std::pair<int, std::span<ShaderModule>> VulkanPlus::GetShaderModule(std::string name)
	{
		return pipeline_manager.GetShaderModule(std::move(name));
	}
	std::pair<int, std::span<ShaderModule>> VulkanPlus::GetShaderModule(int id)
	{
		return pipeline_manager.GetShaderModule(id);
	}
	bool VulkanPlus::HasShaderModule(std::string name)
	{
		return pipeline_manager.HasShaderModule(std::move(name));
	}
	bool VulkanPlus::HasShaderModule(int id)
	{
		return pipeline_manager.HasShaderModule(id);
	}
	size_t VulkanPlus::GetShaderModuleCount() const
	{
		return pipeline_manager.GetShaderModuleCount();
	}

	std::pair<int, std::span<Pipeline>> VulkanPlus::CreatePipeline(std::string name, VkGraphicsPipelineCreateInfo& createInfo)
	{
		return pipeline_manager.CreatePipeline(std::move(name), createInfo);
	}
	std::pair<int, std::span<Pipeline>> VulkanPlus::CreatePipeline(std::string name, VkComputePipelineCreateInfo& createInfo)
	{
		return pipeline_manager.CreatePipeline(std::move(name), createInfo);
	}

	bool VulkanPlus::HasPipeline(std::string name)
	{
		return pipeline_manager.HasPipeline(name);
	}

	bool VulkanPlus::HasPipeline(int id)
	{
		return pipeline_manager.HasPipeline(id);
	}

	int VulkanPlus::DestroyPipeline(std::string name)
	{
		return pipeline_manager.DestroyPipeline(std::move(name));
	}

	int VulkanPlus::DestroyPipeline(int id)
	{
		return pipeline_manager.DestroyPipeline(id);
	}

	std::pair<int, std::span<Pipeline>> VulkanPlus::GetPipeline(std::string name)
	{
		return pipeline_manager.GetPipeline(std::move(name));
	}
	std::pair<int, std::span<Pipeline>> VulkanPlus::GetPipeline(int id)
	{
		return pipeline_manager.GetPipeline(id);
	}
	size_t VulkanPlus::GetPipelineCount() const
	{
		return pipeline_manager.GetPipelineCount();
	}

	std::pair<int, std::span<PipelineLayout>> VulkanPlus::CreatePipelineLayout(std::string name, VkPipelineLayoutCreateInfo& createInfo)
	{
		return pipeline_manager.CreatePipelineLayout(std::move(name), createInfo);
	}
	std::pair<int, std::span<PipelineLayout>> VulkanPlus::GetPipelineLayout(std::string name)
	{
		return pipeline_manager.GetPipelineLayout(std::move(name));
	}
	std::pair<int, std::span<PipelineLayout>> VulkanPlus::GetPipelineLayout(int id)
	{
		return pipeline_manager.GetPipelineLayout(id);
	}
	bool VulkanPlus::HasPipelineLayout(std::string name)
	{
		return pipeline_manager.HasPipelineLayout(std::move(name));
	}
	bool VulkanPlus::HasPipelineLayout(int id)
	{
		return pipeline_manager.HasPipelineLayout(id);
	}
	size_t VulkanPlus::GetPipelineLayoutCount() const
	{
		return pipeline_manager.GetPipelineLayoutCount();
	}
	std::pair<int, std::span<RenderPass>> VulkanPlus::CreateRenderPass(std::string name, VkRenderPassCreateInfo& createInfo)
	{
		return rpwf_manager.CreateRenderPass(std::move(name), createInfo);
	}
	std::pair<int, std::span<RenderPass>> VulkanPlus::GetRenderPass(std::string name)
	{
		return rpwf_manager.GetRenderPass(std::move(name));
	}
	std::pair<int, std::span<RenderPass>> VulkanPlus::GetRenderPass(int id)
	{
		return rpwf_manager.GetRenderPass(id);
	}

	bool VulkanPlus::HasRenderPass(std::string name)
	{
		return rpwf_manager.HasRenderPass(std::move(name));
	}

	bool VulkanPlus::HasRenderPass(int id)
	{
		return rpwf_manager.HasRenderPass(id);
	}

	size_t VulkanPlus::GetRenderPassCount() const
	{
		return rpwf_manager.GetRenderPassCount();
	}

	std::pair<int, std::span<TimestampQueries>> VulkanPlus::CreateTimestampQueries(std::string name, uint32_t count, uint32_t capacity)
	{
		return query_pool_manager.CreateTimestampQueries(std::move(name), count, capacity);
	}

	std::pair<int, std::span<TimestampQueries>> VulkanPlus::GetTimestampQueries(std::string name)
	{
		return query_pool_manager.GetTimestampQueries(std::move(name));
	}

	std::pair<int, std::span<TimestampQueries>> VulkanPlus::GetTimestampQueries(int id)
	{
		return query_pool_manager.GetTimestampQueries(id);
	}

	bool VulkanPlus::HasTimestampQueries(std::string name)
	{
		return query_pool_manager.HasTimestampQueries(std::move(name));
	}

	bool VulkanPlus::HasTimestampQueries(int id)
	{
		return query_pool_manager.HasTimestampQueries(id);
	}

	size_t VulkanPlus::GetTimestampQueriesCount() const
	{
		return query_pool_manager.GetTimestampQueriesCount();
	}

	std::pair<int, std::span<Sampler>> VulkanPlus::CreateSampler(std::string name, VkSamplerCreateInfo& createInfo)
	{
		return sampler_manager.CreateSampler(std::move(name), createInfo);
	}

	std::pair<int, std::span<Sampler>> VulkanPlus::GetSampler(std::string name)
	{
		return sampler_manager.GetSampler(std::move(name));
	}

	std::pair<int, std::span<Sampler>> VulkanPlus::GetSampler(int id)
	{
		return sampler_manager.GetSampler(id);
	}

	bool VulkanPlus::HasSampler(std::string name)
	{
		return sampler_manager.HasSampler(std::move(name));
	}

	bool VulkanPlus::HasSampler(int id)
	{
		return sampler_manager.HasSampler(id);
	}

	size_t VulkanPlus::GetSamplerCount() const
	{
		return sampler_manager.GetSamplerCount();
	}

	std::pair<int, std::span<DescriptorSetLayout>> VulkanPlus::CreateDescriptorSetLayout(std::string name, VkDescriptorSetLayoutCreateInfo& createInfo)
	{
		return descriptor_manager.CreateDescriptorSetLayout(std::move(name), createInfo);
	}
	std::pair<int, std::span<DescriptorSetLayout>> VulkanPlus::GetDescriptorSetLayout(std::string name)
	{
		return descriptor_manager.GetDescriptorSetLayout(std::move(name));
	}
	std::pair<int, std::span<DescriptorSetLayout>> VulkanPlus::GetDescriptorSetLayout(int id)
	{
		return descriptor_manager.GetDescriptorSetLayout(id);
	}
	bool VulkanPlus::HasDescriptorSetLayout(std::string name)
	{
		return descriptor_manager.HasDescriptorSetLayout(std::move(name));
	}
	bool VulkanPlus::HasDescriptorSetLayout(int id)
	{
		return descriptor_manager.HasDescriptorSetLayout(id);
	}
	size_t VulkanPlus::GetDescriptorSetLayoutCount() const
	{
		return descriptor_manager.GetDescriptorSetLayoutCount();
	}

}