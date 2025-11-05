#include "Engine/Panel/Editor/EditorInspectorPanel.h"

namespace HoshioEngine {
	int EditorInspectorPanel::mode = 0;
	EditorInspectorPanel::EditorInspectorPanel(PipelineNode* startNode)
		:startNode(startNode)
	{}

	void EditorInspectorPanel::Render()
	{
		ImGui::Begin("Inspector");
		if (startNode) {
			startNode->ImguiRender();
			auto next = startNode->NextNode();
			while (next) {
				next->ImguiRender();
				next = next->NextNode();
			}
		}
		ImGui::End();
	}
}