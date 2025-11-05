#include "Base/VulkanBase.h"
#include "Engine/ShaderEditor/PipelineNode/PipelineNode.h"

namespace HoshioEngine {

	constexpr VkExtent2D defaultWindowSize = { 1920,1080 };
	inline auto& out = std::cout;

#pragma region VulkanBase

	VulkanBase::~VulkanBase()
	{
		if (!instance)
			return;
		if (device) {
			WaitIdle();
			if (swapchain) {
				//for (auto& func : callbacks_destroySwapchain)
				//	func();
				for (auto& handle : swapchainImageViews)
					if (handle)
						vkDestroyImageView(device, handle, nullptr);
				vkDestroySwapchainKHR(device, swapchain, nullptr);
			}
			for (auto& func : callbacks_destroyDevice)
				func();
			vkDestroyDevice(device, nullptr);
		}
		if (surface)
			vkDestroySurfaceKHR(instance, surface, nullptr);
		if (debugMessenger) {
			PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessenger =
				reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
			if (DestroyDebugUtilsMessenger)
				DestroyDebugUtilsMessenger(instance, debugMessenger, nullptr);
		}
		vkDestroyInstance(instance, nullptr);
	}

	void VulkanBase::AddLayerOrExtension(std::vector<const char*>& container, const char* name)
	{
		container.push_back(name);
	}

	void VulkanBase::UseLastestApiVersion()
	{
		if (vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"))
			vkEnumerateInstanceVersion(&this->apiVersion);
	}

	void VulkanBase::CreateDebugMessenger()
	{
		static PFN_vkDebugUtilsMessengerCallbackEXT DebugUtilsMessengerCallback = [](
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverityFlagBits,
			VkDebugUtilsMessageTypeFlagsEXT messageTypeFlags,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)->VkBool32 {
				std::string severity = "Validation Layer::", type = "";

				if (messageTypeFlags & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
					type += "::GENERAL";
				if (messageTypeFlags & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
					type += "::VALIDATAION";
				if (messageTypeFlags & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
					type += "::PERFORMANCE";
				type += "::";

				if (messageSeverityFlagBits & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
					severity += "[WARNING]";
					COLOR_PRINT(severity + type, OSTREAM_TYPE::CERR, FONT_COLOR::Bright_Yellow);
				}
				else if (messageSeverityFlagBits & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
					severity += "[ERROR]";
					COLOR_PRINT(severity + type, OSTREAM_TYPE::CERR, FONT_COLOR::Red);
				}

				std::cerr << std::endl << pCallbackData->pMessage << std::endl << std::endl;

				return VK_FALSE;
		};
		VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			.pfnUserCallback = DebugUtilsMessengerCallback
		};
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger =
			reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if (vkCreateDebugUtilsMessenger) {
			if (vkCreateDebugUtilsMessenger(instance, &debugUtilsMessengerCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
				throw std::runtime_error("Fail to create debug utils messenger");
			return;
		}
		throw std::runtime_error("Failed to get the function pointer of vkCreateDebugUtilsMessengerEXT!");
	}

	bool VulkanBase::GetQueueFamilyIndices(VkPhysicalDevice physicalDevice, uint32_t(&queueFamilyIndices)[3])
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());
		
		auto& [ig, ip, ic] = queueFamilyIndices;
		ig = ip = ic = VK_QUEUE_FAMILY_IGNORED;
		
		for (uint32_t i = 0; i < queueFamilyCount; i++) {
			VkBool32
				supportGraphics = queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT,
				supportPresent = false,
				supportCompute = queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT;
			if (this->surface)
				if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportPresent) != VK_SUCCESS)
					throw std::runtime_error("Failed to determine if the queue family support presentation.");

			if (supportGraphics && supportPresent && supportCompute) {
				ig = ip = ic = i;
				break;
			}

			if (supportGraphics && supportPresent)
				ig = ip = i;

			if (supportGraphics && ig == VK_QUEUE_FAMILY_IGNORED)
				ig = i;

			if (supportPresent && ip == VK_QUEUE_FAMILY_IGNORED)
				ip = i;

			if (supportCompute && ic == VK_QUEUE_FAMILY_IGNORED)
				ic = i;
		}

		if (ig == VK_QUEUE_FAMILY_IGNORED ||
			ip == VK_QUEUE_FAMILY_IGNORED ||
			ic == VK_QUEUE_FAMILY_IGNORED)
			return false;

		return true;
	}

	void VulkanBase::CreateSwapchain_Internal()
	{
		if (vkCreateSwapchainKHR(device, &swapchainCi, nullptr, &swapchain) != VK_SUCCESS) 
			throw std::runtime_error("Failed to create a swapchain!");

		uint32_t swapchainImageCount;

		if (vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, nullptr) != VK_SUCCESS) 
			throw std::runtime_error("Failed to get the count of swapchain images!");

		swapchainImages.resize(swapchainImageCount);

		if (VkResult result = vkGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages.data())) 
			throw std::runtime_error("Failed to get swapchain images!");

		swapchainImageViews.resize(swapchainImageCount);
		VkImageViewCreateInfo imageViewCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = swapchainCi.imageFormat,
			//.components = {},
			.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
		};
		for (size_t i = 0; i < swapchainImageCount; i++) {
			imageViewCreateInfo.image = swapchainImages[i];
			if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create a swapchain image view!");
			}
		}
	}

	void VulkanBase::AddInstanceLayer(const char* layerName)
	{
		this->AddLayerOrExtension(instanceLayers, layerName);
	}

	void VulkanBase::AddInstanceExtension(const char* extensionName)
	{
		this->AddLayerOrExtension(instanceExtensions, extensionName);
	}

	void VulkanBase::CreateInstance(VkInstanceCreateFlags flags){
#ifndef NDEBUG
		AddInstanceLayer("VK_LAYER_KHRONOS_validation");
		AddInstanceExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		VkApplicationInfo applicationInfo = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.apiVersion = this->apiVersion
		};
		VkInstanceCreateInfo instanceCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.flags = flags,
			.pApplicationInfo = &applicationInfo,
			.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size()),
			.ppEnabledLayerNames = instanceLayers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
			.ppEnabledExtensionNames = instanceExtensions.data()
		};
		if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("Fail to create a vulkan instance!");
		}
		
		out << std::format(
			"Vulkan API Version: {}.{}.{}\n",
			VK_VERSION_MAJOR(apiVersion),
			VK_VERSION_MINOR(apiVersion),
			VK_VERSION_PATCH(apiVersion)
		);

#ifndef NDEBUG
		CreateDebugMessenger();
#endif
	}

	bool VulkanBase::CheckInstanceLayers() const
	{
		uint32_t instanceLayerCount = 0;
		if (vkEnumerateInstanceLayerProperties(&instanceLayerCount,nullptr) != VK_SUCCESS)
			throw std::runtime_error("Fail to get the count of instance layers");
		if (!instanceLayerCount)
			throw std::runtime_error("Fail to get any instance layers");
		std::vector<VkLayerProperties> availableLayerProperties(instanceLayerCount);
		if (vkEnumerateInstanceLayerProperties( &instanceLayerCount, availableLayerProperties.data()) != VK_SUCCESS)
			throw std::runtime_error("Fail to get instance layers");

		for (auto& m_layer : instanceLayers) {
			bool found = false;
			for (auto& layer : availableLayerProperties) {
				if (strcmp(layer.layerName, m_layer) == 0) {
					found = true;
					break;
				}
			}
			if (!found) {
				std::string error_info = std::string("Fail to find layer : ") + std::string(m_layer);
				throw std::runtime_error(error_info);
			}
		}
		return true;
	}

	void VulkanBase::AddDeviceExtension(const char* extensionName)
	{
		this->deviceExtensions.push_back(extensionName);
	}

	void VulkanBase::GetPhysicalDevice()
	{
		uint32_t physicalDeviceCount = 0;

		if(vkEnumeratePhysicalDevices(this->instance, &physicalDeviceCount, nullptr) != VK_SUCCESS)
			throw std::runtime_error("Failed to get the count of physical devices");

		if (physicalDeviceCount == 0)
			throw std::runtime_error("Failed to get any physical device");
		this->availablePhysicalDevices.resize(physicalDeviceCount);

		if (vkEnumeratePhysicalDevices(this->instance, &physicalDeviceCount, this->availablePhysicalDevices.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to get physical devices");
	}

	void VulkanBase::DeterminePhysicalDevice()
	{
		for (int i = 0; i < availablePhysicalDevices.size(); i++) {
			uint32_t queueIndices[3];
			if (!GetQueueFamilyIndices(availablePhysicalDevices[i], queueIndices))
				continue;

			auto& [ig, ip, ic] = queueIndices;
			this->queueFamilyIndex_graphics = ig;
			this->queueFamilyIndex_present = ip;
			this->queueFamilyIndex_compute = ic;
			this->physicalDevice = availablePhysicalDevices[i];

			if (ig == ip && ip == ic) 
				break;
		}
	}

	void VulkanBase::CreateDevice(VkDeviceCreateFlags flags)
	{
		float queuePriority = 1.f;
		VkDeviceQueueCreateInfo queueCreateInfos[3] = {
			{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority },
			{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority },
			{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueCount = 1,
				.pQueuePriorities = &queuePriority } };
		uint32_t queueCreateInfoCount = 0;
		if (queueFamilyIndex_graphics != VK_QUEUE_FAMILY_IGNORED)
			queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndex_graphics;
		if (queueFamilyIndex_present != VK_QUEUE_FAMILY_IGNORED &&
			queueFamilyIndex_present != queueFamilyIndex_graphics)
			queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndex_present;
		if (queueFamilyIndex_compute != VK_QUEUE_FAMILY_IGNORED &&
			queueFamilyIndex_compute != queueFamilyIndex_graphics &&
			queueFamilyIndex_compute != queueFamilyIndex_present)
			queueCreateInfos[queueCreateInfoCount++].queueFamilyIndex = queueFamilyIndex_compute;
		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
		VkPhysicalDeviceDescriptorIndexingFeatures indexing{};
		indexing.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		indexing.descriptorBindingPartiallyBound = VK_TRUE;
		VkDeviceCreateInfo deviceCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &indexing,
			.flags = flags,
			.queueCreateInfoCount = queueCreateInfoCount,
			.pQueueCreateInfos = queueCreateInfos,
			.enabledExtensionCount = uint32_t(deviceExtensions.size()),
			.ppEnabledExtensionNames = deviceExtensions.data(),
			.pEnabledFeatures = &physicalDeviceFeatures
		};
		if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) 
			throw std::runtime_error("Fail to create a vulkan logical device!");
		if (queueFamilyIndex_graphics != VK_QUEUE_FAMILY_IGNORED)
			vkGetDeviceQueue(device, queueFamilyIndex_graphics, 0, &queue_graphics);
		if (queueFamilyIndex_present != VK_QUEUE_FAMILY_IGNORED)
			vkGetDeviceQueue(device, queueFamilyIndex_present, 0, &queue_present);
		if (queueFamilyIndex_compute != VK_QUEUE_FAMILY_IGNORED)
			vkGetDeviceQueue(device, queueFamilyIndex_compute, 0, &queue_compute);
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
		out << std::format("Renderer: {}\n", physicalDeviceProperties.deviceName);
		for (auto& func : callbacks_createDevice)
			func();
	}

	void VulkanBase::GetSurfaceFormats()
	{
		uint32_t surfaceFormatCount = 0;
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, this->surface, &surfaceFormatCount, nullptr) != VK_SUCCESS)
			throw std::runtime_error("Fail to get the count of surface format count");
		if (!surfaceFormatCount)
			throw std::runtime_error("Fail to get any surface format");
		availableSurfaceFormats.resize(surfaceFormatCount);
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice, this->surface, &surfaceFormatCount, availableSurfaceFormats.data()) != VK_SUCCESS)
			throw std::runtime_error("Fail to get suface format");
	}

	bool VulkanBase::SetSurfaceFormat(VkSurfaceFormatKHR surfaceFormat)
	{
		bool formatIsAvailable = false;
		if (!surfaceFormat.format) {
			for (auto& i : availableSurfaceFormats)
				if (i.colorSpace == surfaceFormat.colorSpace) {
					swapchainCi.imageFormat = i.format;
					swapchainCi.imageColorSpace = i.colorSpace;
					formatIsAvailable = true;
					break;
				}
		}
		else {
			for (auto& i : availableSurfaceFormats)
				if (i.format == surfaceFormat.format &&
					i.colorSpace == surfaceFormat.colorSpace) {
					swapchainCi.imageFormat = i.format;
					swapchainCi.imageColorSpace = i.colorSpace;
					formatIsAvailable = true;
					break;
				}
		}
		if (!formatIsAvailable)
			return false;
		if (swapchain)
			RecreateSwapchain();
		return true;
	}

	void VulkanBase::CreateSwapchain(bool limitFrameRate, VkSwapchainCreateFlagsKHR flags )
	{
		//Get surface capabilities
		VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities) != VK_SUCCESS) 
			throw std::runtime_error("Fail to get physical device surface capabilities!");
		minImageCount = surfaceCapabilities.minImageCount;
		//Set image count
		swapchainCi.minImageCount = surfaceCapabilities.minImageCount + (surfaceCapabilities.maxImageCount > surfaceCapabilities.minImageCount);
		//Set image extent
		swapchainCi.imageExtent =
			surfaceCapabilities.currentExtent.width == -1 ?
			VkExtent2D{
			glm::clamp(defaultWindowSize.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width),
			glm::clamp(defaultWindowSize.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height) } :
			surfaceCapabilities.currentExtent;
		//Set transformation
		swapchainCi.preTransform = surfaceCapabilities.currentTransform;
		//Set alpha compositing mode
		if (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
			swapchainCi.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
		else
			for (size_t i = 0; i < 4; i++)
				if (surfaceCapabilities.supportedCompositeAlpha & 1 << i) {
					swapchainCi.compositeAlpha = VkCompositeAlphaFlagBitsKHR(surfaceCapabilities.supportedCompositeAlpha & 1 << i);
					break;
				}
		//Set image usage
		swapchainCi.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
			swapchainCi.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			swapchainCi.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		else
			out << std::format("[ graphicsBase ] WARNING\nVK_IMAGE_USAGE_TRANSFER_DST_BIT isn't supported!\n");

		//Get surface formats
		if (!availableSurfaceFormats.size())
			GetSurfaceFormats();
		//If surface format is not determined, select a a four-component UNORM format
		if (!swapchainCi.imageFormat) {
			if (!SetSurfaceFormat({ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }) &&
				!SetSurfaceFormat({ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })) {
				swapchainCi.imageFormat = availableSurfaceFormats[0].format;
				swapchainCi.imageColorSpace = availableSurfaceFormats[0].colorSpace;
				out << std::format("[ graphicsBase ] WARNING\nFailed to select a four-component UNORM surface format!\n");
			}
		}
		//Get surface present modes
		uint32_t surfacePresentModeCount;
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModeCount, nullptr) != VK_SUCCESS) 
			throw std::runtime_error("Failed to get the count of surface present modes!");

		if (!surfacePresentModeCount)
			throw std::runtime_error("Failed to find any surface present mode!");
		std::vector<VkPresentModeKHR> surfacePresentModes(surfacePresentModeCount);
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModeCount, surfacePresentModes.data()) != VK_SUCCESS) 
			throw std::runtime_error("Failed to get surface present modes!");
		//Set present mode to mailbox if available and necessary
		swapchainCi.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		if (!limitFrameRate)
			for (size_t i = 0; i < surfacePresentModeCount; i++)
				if (surfacePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
					swapchainCi.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}

		swapchainCi.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCi.flags = flags;
		swapchainCi.surface = surface;
		swapchainCi.imageArrayLayers = 1;
		swapchainCi.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCi.clipped = VK_TRUE;

		CreateSwapchain_Internal();

		for (auto& func : callbacks_createSwapchain)
			func();
	}

	void VulkanBase::RecreateSwapchain(VkSwapchainCreateFlagsKHR flags)
	{
		VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities) != VK_SUCCESS) 
			throw std::runtime_error("Failed to get physical device surface capabilities!");

		if (surfaceCapabilities.currentExtent.width == 0 ||
			surfaceCapabilities.currentExtent.height == 0)
			return;

		swapchainCi.imageExtent = surfaceCapabilities.currentExtent;
		swapchainCi.oldSwapchain = swapchain;

		vkQueueWaitIdle(queue_graphics);
		if (queue_graphics != queue_present)
			vkQueueWaitIdle(queue_present);

		for (auto& func : callbacks_destroySwapchain)
			func();
		for (auto& imageView : swapchainImageViews)
			if (imageView)
				vkDestroyImageView(device, imageView, nullptr);
		swapchainImageViews.resize(0);
		CreateSwapchain_Internal();
		for (auto& func : callbacks_createSwapchain)
			func();
	}

	void VulkanBase::AddCallback_CreateSwapchain(std::function<void()> func)
	{
		callbacks_createSwapchain.push_back(func);
	}

	void VulkanBase::AddCallback_DestroySwapchain(std::function<void()> func)
	{
		callbacks_destroySwapchain.push_back(func);
	}

	void VulkanBase::AddCallback_CreateDevice(std::function<void()> func)
	{
		callbacks_createDevice.push_back(func);
	}

	void VulkanBase::AddCallBack_DestroyDevice(std::function<void()> func)
	{
		callbacks_destroyDevice.push_back(func);
	}

	void VulkanBase::WaitIdle() const
	{
		if (vkDeviceWaitIdle(device) != VK_SUCCESS)
			throw std::runtime_error("Failed to wait for device into idle!");
	}

	void VulkanBase::SubmitCommandBuffer_Graphics(VkSubmitInfo& submitInfo, VkFence fence)
	{
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		if (vkQueueSubmit(queue_graphics, 1, &submitInfo, fence) != VK_SUCCESS)
			throw std::runtime_error("Failed to submit command buffer to graphics queue");
	}

	void VulkanBase::SubmitCommandBuffer_Graphics(ArrayRef<VkCommandBuffer> commandBuffers, VkFence fence)
	{
		VkSubmitInfo submitInfo = {
			.commandBufferCount = uint32_t(commandBuffers.size()),
			.pCommandBuffers = commandBuffers.data()
		};
		return SubmitCommandBuffer_Graphics(submitInfo, fence);
	}

	void VulkanBase::SubmitCommandBuffer_Graphics(ArrayRef<VkCommandBuffer> commandBuffers, VkSemaphore semaphore_image_available, VkSemaphore semaphore_render_over, VkFence fence, VkPipelineStageFlags waitDstStage_imageAvailable)
	{
		VkSubmitInfo submitInfo = {
			.commandBufferCount = uint32_t(commandBuffers.size()),
			.pCommandBuffers = commandBuffers.data()
		};
		if (semaphore_image_available) {
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &semaphore_image_available;
			submitInfo.pWaitDstStageMask = &waitDstStage_imageAvailable;
		}
		if (semaphore_image_available) {
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &semaphore_render_over;
		}
		return SubmitCommandBuffer_Graphics(submitInfo, fence);
	}

	void VulkanBase::SubmitCommandBuffer_Compute(VkSubmitInfo& submitInfo, VkFence fence)
	{
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		if (vkQueueSubmit(queue_compute, 1, &submitInfo, fence) != VK_SUCCESS)
			throw std::runtime_error("Failed to submit command buffer to compute queue");
	}

	void VulkanBase::SubmitCommandBuffer_Compute(ArrayRef<VkCommandBuffer> commandBuffers, VkFence fence)
	{
		VkSubmitInfo submitInfo = {
			.commandBufferCount = uint32_t(commandBuffers.size()),
			.pCommandBuffers = commandBuffers.data()
		};
		SubmitCommandBuffer_Compute(submitInfo, fence);
	}

	void VulkanBase::PresentImage(VkPresentInfoKHR& presentInfo)
	{
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		VkResult result = vkQueuePresentKHR(queue_present, &presentInfo);
		switch (result) {
		case VK_SUCCESS:
			break;
		case VK_SUBOPTIMAL_KHR:
		case VK_ERROR_OUT_OF_DATE_KHR:
			RecreateSwapchain();
			break;
		default:
			throw std::runtime_error("Failed to queue the image for presentImage!");
		}
	}

	void VulkanBase::PresentImage(VkSemaphore semaphore_render_over)
	{
		VkPresentInfoKHR presentInfo = {
			.swapchainCount = 1,
			.pSwapchains = &swapchain,
			.pImageIndices = &currentImageIndex
		};
		if (semaphore_render_over) {
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &semaphore_render_over;
		}
		PresentImage(presentInfo);
	}

	VkInstance VulkanBase::Instance() const
	{
		return this->instance;
	}

	uint32_t VulkanBase::ApiVersion() const
	{
		return this->apiVersion;
	}

	const std::vector<const char*>& VulkanBase::InstanceLayers() const
	{
		return this->instanceLayers;
	}

	const std::vector<const char*>& VulkanBase::InstanceExtensions() const
	{
		return this->instanceExtensions;
	}

	VkSurfaceKHR VulkanBase::Surface() const
	{
		return this->surface;
	}

	VkPhysicalDevice VulkanBase::PhysicalDevice() const
	{
		return this->physicalDevice;
	}

	const VkPhysicalDeviceProperties& VulkanBase::PhysicalDeviceProperties() const
	{
		return this->physicalDeviceProperties;
	}

	const VkPhysicalDeviceMemoryProperties& VulkanBase::PhysicalDeviceMemoryProperties() const
	{
		return this->physicalDeviceMemoryProperties;
	}

	VkPhysicalDevice VulkanBase::AvailablePhysicalDevices(uint32_t index) const
	{
		return this->availablePhysicalDevices[index];
	}

	VkDevice VulkanBase::Device() const
	{
		return this->device;
	}

	uint32_t VulkanBase::QueueFamilyIndex_Graphics() const
	{
		return this->queueFamilyIndex_graphics;
	}

	uint32_t VulkanBase::QueueFamilyIndex_Present() const
	{
		return this->queueFamilyIndex_present;
	}

	uint32_t VulkanBase::QueueFamilyIndex_Compute() const
	{
		return this->queueFamilyIndex_compute;
	}

	VkQueue VulkanBase::Queue_Graphics() const
	{
		return this->queue_graphics;
	}

	VkQueue VulkanBase::Queue_Present() const
	{
		return this->queue_present;
	}

	VkQueue VulkanBase::Queue_compute() const
	{
		return this->queue_compute;
	}

	const VkFormat& VulkanBase::AvailableSurfaceFormat(uint32_t index) const
	{
		return this->availableSurfaceFormats[index].format;
	}

	const VkColorSpaceKHR& VulkanBase::AvailableSurfaceColorSpace(uint32_t index) const
	{
		return this->availableSurfaceFormats[index].colorSpace;
	}

	uint32_t VulkanBase::AvailableSurfaceFormatCount() const
	{
		return static_cast<uint32_t>(this->availableSurfaceFormats.size());
	}

	VkSwapchainKHR VulkanBase::Swapchain() const
	{
		return this->swapchain;
	}

	VkImage VulkanBase::SwapchainImage(uint32_t index) const
	{
		return this->swapchainImages[index];
	}

	VkImageView VulkanBase::SwapchainImageView(uint32_t index) const
	{
		return this->swapchainImageViews[index];
	}

	uint32_t VulkanBase::SwapchainImageCount() const
	{
		return static_cast<uint32_t>(this->swapchainImages.size());
	}

	VkExtent2D VulkanBase::SwapchainExtent() const
	{
		return this->swapchainCi.imageExtent;
	}

	const VkSwapchainCreateInfoKHR& VulkanBase::SwapchainCi() const
	{
		return this->swapchainCi;
	}

	float VulkanBase::AspectRatio() const
	{
		return 1.0f * swapchainCi.imageExtent.width / swapchainCi.imageExtent.height;
	}

	uint32_t VulkanBase::CurrentImageIndex() const
	{
		return this->currentImageIndex;
	}

	uint32_t VulkanBase::MinImageCount() const
	{
		return minImageCount;
	}

	void VulkanBase::SwapImage(VkSemaphore semaphore_image_available)
	{
		if (swapchainCi.oldSwapchain &&
			swapchainCi.oldSwapchain != swapchain) {
			vkDestroySwapchainKHR(device, swapchainCi.oldSwapchain, nullptr);
			swapchainCi.oldSwapchain = VK_NULL_HANDLE;
		}
		while (VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore_image_available, VK_NULL_HANDLE, &currentImageIndex)) {
			switch (result) {
			case VK_SUBOPTIMAL_KHR:
			case VK_ERROR_OUT_OF_DATE_KHR:
				RecreateSwapchain();
				break;
			default:
				throw std::runtime_error("Failed to acquire the next image!");
			}
		}
	}

	void VulkanBase::Surface(VkSurfaceKHR surface)
	{
		this->surface = surface;
	}

	VulkanBase& VulkanBase::Base()
	{
		static VulkanBase vulkanBase;
		return vulkanBase;
	}

#pragma endregion

}
