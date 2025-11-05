#ifndef _EDITOR_GUI_MANAGER_H_
#define _EDITOR_GUI_MANAGER_H_

#include "Engine/Panel/Editor/EditorPanel.h"
#include "Base/DescriptorManager.h"
#include "Base/RpwfManager.h"
#include "Base/CommandManager.h"
#include "Wins/GlfwManager.h"

namespace HoshioEngine {
	class EditorGUIManager {
	public:
		static EditorGUIManager& Instance();

		void Render();
		const CommandBuffer& GCommandBuffer() const;

		std::vector<std::unique_ptr<EditorPanel>> editorPanels;
	private:
		EditorGUIManager();
		EditorGUIManager(EditorGUIManager&& editorGUIManager) = delete;
		EditorGUIManager(const EditorGUIManager& other) = delete;
		EditorGUIManager& operator=(const EditorGUIManager& other) = delete;
		~EditorGUIManager();

		void BeginEditorRender();
		void EditorRender();
		void EndEditorRender();

		void InitForVulkan();
		void FrameRender();


		DescriptorPool g_DescriptorPool;
		RenderPass g_RenderPass;
		CommandPool g_CommandPool;
		CommandBuffer g_CommandBuffer;
		std::vector<Framebuffer> g_Framebuffers;

	};
}


#endif // !_IMGUI_ENGINE_H_
