#include "Wins/GlfwManager.h"


namespace HoshioEngine {

	GLFWwindow* GlfwWindow::pWindow = nullptr;
	GLFWmonitor* GlfwWindow::pMonitor = nullptr;
	const char* GlfwWindow::windowTitle = "HoshioEngine";
	double GlfwWindow::deltaTime = 0.0;
	Camera GlfwWindow::camera;
	bool GlfwWindow::is3d = false;

	bool GlfwWindow::InitializeWindow(VkExtent2D size, bool fullScreen, bool isResizable, bool limitFrameRate)
	{
		static GlfwWindow glfwWindow;

		if (!glfwInit()) {
			std::cout << std::format("[ InitializeWindow ] ERROR\nFailed to initialize GLFW!\n");
			return false;
		}
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, isResizable);

		pMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);
		pWindow = fullScreen ?
			glfwCreateWindow(pMode->width, pMode->height, windowTitle, pMonitor, nullptr):
			glfwCreateWindow(size.width, size.height, windowTitle, nullptr, nullptr);

		if (!pWindow) {
			std::cout << std::format("[ InitializeWindow ]\nFailed to create a glfw window!\n");
			return false;
		}

		glfwSetCursorPosCallback(pWindow, MouseCallback);
		glfwSetScrollCallback(pWindow, ScrollCallback);
		glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		uint32_t extensionCount = 0;
		const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
		if (!extensions) {
			std::cout << std::format("[ InitializeWindow ]\n Failed to get required glfw instance extensions!\n");
			return false;
		}
		for (uint32_t i = 0; i < extensionCount; i++) 
			VulkanBase::Base().AddInstanceExtension(extensions[i]);

		VulkanBase::Base().AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		VulkanBase::Base().AddDeviceExtension(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
		VulkanBase::Base().UseLastestApiVersion();
		
		VulkanBase::Base().CreateInstance();

		VkSurfaceKHR surface = VK_NULL_HANDLE;
		if (glfwCreateWindowSurface(VulkanBase::Base().Instance(), pWindow, nullptr, &surface) != VK_SUCCESS) {
			std::cout << std::format("[ InitialIzeWindow ]\n Failed to create a window surface!");
			return false;
		}
		VulkanBase::Base().Surface(surface);

		VulkanBase::Base().GetPhysicalDevice();
		VulkanBase::Base().DeterminePhysicalDevice();
		VulkanBase::Base().CreateDevice();

		VulkanBase::Base().CreateSwapchain();

		return true;
	}

	GlfwWindow::~GlfwWindow()
	{
		TerminateWindow();
	}

	void GlfwWindow::TitleFps()
	{
		static double time0 = glfwGetTime();
		static double time1;
		static double dt;
		static int dframe = -1;
		static std::stringstream info;
		time1 = glfwGetTime();
		dframe++;
		if ((dt = time1 - time0) >= 1) {
			info.precision(1);
			info << windowTitle << "    " << std::fixed << dframe / dt << " FPS";
			glfwSetWindowTitle(pWindow, info.str().c_str());
			info.str("");
			time0 = time1;
			dframe = 0;
		}
	}

	void GlfwWindow::CalDeltaTime()
	{
		static double lastFrameTime = 0.0;
		static double currentFrameTime = 0.0;
		currentFrameTime = glfwGetTime();
		deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;
	}

	void GlfwWindow::ProcessInput()
	{
		if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(pWindow, true);

		if (glfwGetKey(pWindow, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
			glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		if (glfwGetKey(pWindow, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE) {
			glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}

		if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(CAMERA_MOVEMENT::FORWARD, deltaTime);
		if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(CAMERA_MOVEMENT::BACKWARD, deltaTime);
		if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(CAMERA_MOVEMENT::LEFT, deltaTime);
		if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(CAMERA_MOVEMENT::RIGHT, deltaTime);
		if (glfwGetKey(pWindow, GLFW_KEY_Q) == GLFW_PRESS)
			camera.ProcessKeyboard(CAMERA_MOVEMENT::UP, deltaTime);
		if (glfwGetKey(pWindow, GLFW_KEY_E) == GLFW_PRESS)
			camera.ProcessKeyboard(CAMERA_MOVEMENT::DOWN, deltaTime);

	}

	void GlfwWindow::MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
	{
		static bool firstMouse = true;
		static float lastX = 0.0f;
		static float lastY = 0.0f;

		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

		lastX = xpos;
		lastY = ypos;

		if (glfwGetInputMode(pWindow, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
			firstMouse = true;
			return;
		}
		camera.ProcessMouseMovement(xoffset, yoffset);
	}

	void GlfwWindow::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		if (glfwGetInputMode(pWindow, GLFW_CURSOR) == GLFW_CURSOR_NORMAL) {
			return;
		}
		camera.ProcessMouseScroll(static_cast<float>(yoffset));
	}

	void GlfwWindow::TerminateWindow()
	{
		glfwTerminate();
	}

	void GlfwWindow::UpdateWindow() {
		CalDeltaTime();
		TitleFps();
		ProcessInput();
	}

}
