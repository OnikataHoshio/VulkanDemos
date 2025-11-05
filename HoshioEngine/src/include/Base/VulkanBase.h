#ifndef _VULKAN_BASE_H_
#define _VULKAN_BASE_H_

#include "Utils/CommonUtils.h"

namespace HoshioEngine {
	class VulkanBase {
	public:
		//VkInstance
		void AddInstanceLayer(const char* layerName);

		void AddInstanceExtension(const char* extensionName);

		void CreateInstance(VkInstanceCreateFlags flags = 0);

		bool CheckInstanceLayers() const;

		void UseLastestApiVersion();

		//VkDevice
		void AddDeviceExtension(const char* extensionName);

		void GetPhysicalDevice();

		void DeterminePhysicalDevice();

		void CreateDevice(VkDeviceCreateFlags flags = 0);

		//VkSwapchain
		void GetSurfaceFormats();

		bool SetSurfaceFormat(VkSurfaceFormatKHR surfaceFormat);

		void CreateSwapchain(bool limitFrameRate = true, VkSwapchainCreateFlagsKHR flags = 0);

		void RecreateSwapchain(VkSwapchainCreateFlagsKHR flags = 0);

		//Others
		void AddCallback_CreateSwapchain(std::function<void()> func);
		
		void AddCallback_DestroySwapchain(std::function<void()> func);

		void AddCallback_CreateDevice(std::function<void()> func);

		void AddCallBack_DestroyDevice(std::function<void()> func);

		void WaitIdle() const;

		void SubmitCommandBuffer_Graphics(VkSubmitInfo& submitInfo, VkFence fence = VK_NULL_HANDLE);

		void SubmitCommandBuffer_Graphics(ArrayRef<VkCommandBuffer> commandBuffers, VkFence fence = VK_NULL_HANDLE);

		void SubmitCommandBuffer_Graphics(ArrayRef<VkCommandBuffer> commandBuffers,
			VkSemaphore semaphore_image_available = VK_NULL_HANDLE, VkSemaphore semaphore_render_over = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE,
			VkPipelineStageFlags waitDstStage_imageAvailable = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		void SubmitCommandBuffer_Compute(VkSubmitInfo& submitInfo, VkFence fence = VK_NULL_HANDLE);

		void SubmitCommandBuffer_Compute(ArrayRef<VkCommandBuffer> commandBuffers, VkFence fence = VK_NULL_HANDLE);

		void PresentImage(VkPresentInfoKHR& presentInfo);

		void PresentImage(VkSemaphore semaphore_render_over = VK_NULL_HANDLE);

		//Getter
		VkInstance Instance() const;

		uint32_t ApiVersion() const;

		const std::vector<const char*>& InstanceLayers() const;

		const std::vector<const char*>& InstanceExtensions() const;

		VkSurfaceKHR Surface() const;

		VkPhysicalDevice PhysicalDevice() const;

		const VkPhysicalDeviceProperties& PhysicalDeviceProperties() const;

		const VkPhysicalDeviceMemoryProperties& PhysicalDeviceMemoryProperties() const;

		VkPhysicalDevice AvailablePhysicalDevices(uint32_t index) const;

		VkDevice Device() const;

		uint32_t QueueFamilyIndex_Graphics() const;

		uint32_t QueueFamilyIndex_Present() const;

		uint32_t QueueFamilyIndex_Compute() const;

		VkQueue Queue_Graphics() const;

		VkQueue Queue_Present() const;

		VkQueue Queue_compute() const;
		
		const VkFormat& AvailableSurfaceFormat(uint32_t index) const;
		
		const VkColorSpaceKHR& AvailableSurfaceColorSpace(uint32_t index) const;

		uint32_t AvailableSurfaceFormatCount() const;

		VkSwapchainKHR Swapchain() const;

		VkImage SwapchainImage(uint32_t index) const;

		VkImageView SwapchainImageView(uint32_t index) const;

		uint32_t SwapchainImageCount() const;

		VkExtent2D SwapchainExtent() const;

		const VkSwapchainCreateInfoKHR& SwapchainCi() const;

		float AspectRatio() const;

		uint32_t CurrentImageIndex() const;

		uint32_t MinImageCount() const;

		void SwapImage(VkSemaphore semaphore_image_available);

		//Setter
		void Surface(VkSurfaceKHR surface);

		//Static
		static VulkanBase& Base();

	private:
		VulkanBase() = default;
		VulkanBase(VulkanBase&&) = delete;
		VulkanBase(const VulkanBase&) = delete;
		VulkanBase& operator=(const VulkanBase&) = delete;
		~VulkanBase();

		//VkInstance
		VkInstance instance;
		uint32_t apiVersion = VK_API_VERSION_1_0;
		std::vector<const char*> instanceLayers;
		std::vector<const char*> instanceExtensions;
		
		void AddLayerOrExtension(std::vector<const char*>& container, const char* name);

		//VkDebugMessenger
		VkDebugUtilsMessengerEXT debugMessenger;

		void CreateDebugMessenger();

		//VkSurfaceKHR
		VkSurfaceKHR surface;

		//VkDevice
		VkPhysicalDevice physicalDevice;
		VkPhysicalDeviceProperties physicalDeviceProperties;
		VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
		std::vector<VkPhysicalDevice> availablePhysicalDevices;

		VkDevice device;
		uint32_t queueFamilyIndex_graphics = VK_QUEUE_FAMILY_IGNORED;
		uint32_t queueFamilyIndex_present = VK_QUEUE_FAMILY_IGNORED;
		uint32_t queueFamilyIndex_compute = VK_QUEUE_FAMILY_IGNORED;
		VkQueue queue_graphics;
		VkQueue queue_present;
		VkQueue queue_compute;

		std::vector<const char*> deviceExtensions;

		bool GetQueueFamilyIndices(VkPhysicalDevice physicalDevice, uint32_t(&queueFamilyIndices)[3]);

		//VkSwapchain
		std::vector<VkSurfaceFormatKHR> availableSurfaceFormats;

		VkSwapchainKHR swapchain;
		std::vector <VkImage> swapchainImages;
		std::vector <VkImageView> swapchainImageViews;
		VkSwapchainCreateInfoKHR swapchainCi = {};

		uint32_t currentImageIndex = 0;
		uint32_t minImageCount = 0;

		void CreateSwapchain_Internal();

		//others
		std::vector<std::function<void()>> callbacks_destroySwapchain;
		std::vector<std::function<void()>> callbacks_createSwapchain;
		std::vector<std::function<void()>> callbacks_createDevice;
		std::vector<std::function<void()>> callbacks_destroyDevice;

	};
}

#endif // !_VULKAN_BASE_H_

