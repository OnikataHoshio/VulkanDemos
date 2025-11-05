#include "Engine/Panel/Editor/EditorGUIManager.h"

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "ImGui Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

namespace HoshioEngine {

	EditorGUIManager::EditorGUIManager()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsLight();

		InitForVulkan();
	}

	EditorGUIManager::~EditorGUIManager()
	{
		VulkanBase::Base().WaitIdle();
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	EditorGUIManager& EditorGUIManager::Instance()
	{
		static EditorGUIManager editorGUIManager;
		return editorGUIManager;
	}

	void EditorGUIManager::Render() 
	{
		BeginEditorRender();
		EditorRender();
		EndEditorRender();
	}

	const CommandBuffer& EditorGUIManager::GCommandBuffer() const
	{
		return g_CommandBuffer;
	}

	void EditorGUIManager::BeginEditorRender()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}
	void EditorGUIManager::EditorRender()
	{
		{
			
			for (auto& panel : editorPanels)
				panel->Render();
		}
		ImGui::Render();
	}
	void EditorGUIManager::EndEditorRender()
	{
		FrameRender();
	}
	void EditorGUIManager::InitForVulkan()
	{
		{
			VkDescriptorPoolSize pool_sizes[] =
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
			g_DescriptorPool.Create(1000 * XS_ARRAYSIZE(pool_sizes), pool_sizes, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
		}

		{
			VkAttachmentDescription colorAttachment = {
					.format =VulkanBase::Base().SwapchainCi().imageFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			};

			VkAttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass = {
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments = &colorAttachmentRef
			};

			VkSubpassDependency dependency = {
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = 0,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
			};

			VkRenderPassCreateInfo renderPassCreateInfo = {
				.attachmentCount = 1,
				.pAttachments = &colorAttachment,
				.subpassCount = 1,
				.pSubpasses = &subpass,
				.dependencyCount = 1,
				.pDependencies = &dependency
			};

			g_RenderPass.Create(renderPassCreateInfo);
		}

		{
			auto Create = [&] {
				g_Framebuffers.resize(VulkanBase::Base().SwapchainImageCount());
				for (uint32_t i = 0; i < VulkanBase::Base().SwapchainImageCount(); i++) {
					VkFramebufferCreateInfo framebufferCreateInfo = {
						.renderPass = g_RenderPass,
						.attachmentCount = 1,
						.width = VulkanBase::Base().SwapchainCi().imageExtent.width,
						.height = VulkanBase::Base().SwapchainCi().imageExtent.height,
						.layers = 1
					};
					VkImageView attachment = VulkanBase::Base().SwapchainImageView(i);
					framebufferCreateInfo.pAttachments = &attachment;
					g_Framebuffers[i].Create(framebufferCreateInfo);
				}
			};

			auto Destroy = [&] {
				g_Framebuffers.clear();
			};

			Create();
			VulkanBase::Base().AddCallback_CreateSwapchain(Create);
			VulkanBase::Base().AddCallback_DestroySwapchain(Destroy);
		}

		{
			g_CommandPool.Create(VulkanBase::Base().QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
			g_CommandPool.Allocate(g_CommandBuffer);
		}

		ImGui_ImplGlfw_InitForVulkan(GlfwWindow::pWindow, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = VulkanBase::Base().Instance();
		init_info.PhysicalDevice = VulkanBase::Base().PhysicalDevice();
		init_info.Device = VulkanBase::Base().Device();
		init_info.QueueFamily = VulkanBase::Base().QueueFamilyIndex_Graphics();
		init_info.Queue = VulkanBase::Base().Queue_Graphics();
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = g_DescriptorPool;
		init_info.RenderPass = g_RenderPass;
		init_info.Subpass = 0;
		init_info.MinImageCount = VulkanBase::Base().MinImageCount();
		init_info.ImageCount = VulkanBase::Base().SwapchainImageCount();
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = VK_NULL_HANDLE;
		init_info.CheckVkResultFn = check_vk_result;

		ImGui_ImplVulkan_Init(&init_info);

		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->AddFontFromFileTTF("res/fonts/msyh.ttc", 24.0f, nullptr, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
		ImGui_ImplVulkan_CreateFontsTexture();
	}

	void EditorGUIManager::FrameRender()
	{
		g_CommandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		g_RenderPass.Begin(g_CommandBuffer, g_Framebuffers[VulkanBase::Base().CurrentImageIndex()], { {},VulkanBase::Base().SwapchainCi().imageExtent }, {});
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), g_CommandBuffer);
		g_RenderPass.End(g_CommandBuffer);
		g_CommandBuffer.End();
	}

}