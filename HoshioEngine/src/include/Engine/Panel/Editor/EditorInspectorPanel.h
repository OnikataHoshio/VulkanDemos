#ifndef _EDIOTR_INSPECTOR_PANEL_H_
#define _EDIOTR_INSPECTOR_PANEL_H_

#include "Engine/Panel/Editor/EditorPanel.h"
namespace HoshioEngine {
	class EditorInspectorPanel : public EditorPanel {
	private:
		PipelineNode* startNode = nullptr;
	public:
		static int mode;

		EditorInspectorPanel() = default;
		EditorInspectorPanel(PipelineNode* startNode);
		~EditorInspectorPanel() = default;

		void Render() override;

	};
}

#endif // !_EDIOTR_INSPECTOR_PANEL_HPP_