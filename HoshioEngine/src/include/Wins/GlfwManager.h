#ifndef _GLFW_MANAGER_H_
#define _GLFW_MANAGER_H_

#define GLFW_INCLUDE_VULKAN
#include "Base/VulkanBase.h"
#include "Engine/Object/Camera.h"
#include <GLFW/glfw3.h>

namespace HoshioEngine {
	class GlfwWindow {
	public:
		//static
		static GLFWwindow* pWindow;
		static GLFWmonitor* pMonitor;
		static const char* windowTitle;
		static double deltaTime;
		static Camera camera;
		static bool is3d;

		static bool InitializeWindow(VkExtent2D size, bool fullScreen = false, bool isResizable = true, bool limitFrameRate = true);
		static void UpdateWindow();

	private:
		GlfwWindow() = default;
		GlfwWindow(GlfwWindow&&) = delete;
		GlfwWindow(const GlfwWindow&) = delete;
		GlfwWindow& operator=(const GlfwWindow&) = delete;
		~GlfwWindow();

		void TerminateWindow();
		//static
		static void TitleFps();
		static void CalDeltaTime();
		static void ProcessInput();

		static void MouseCallback(GLFWwindow* window, double xposIn, double yposIn);
		static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	};
}

#endif // !_GLFW_MANAGER_H_

