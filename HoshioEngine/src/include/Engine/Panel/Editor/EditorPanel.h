#ifndef _EDITOR_PANEL_H_
#define _EDITOR_PANEL_H_

#include "Engine/ShaderEditor/PipelineNode/PipelineNode.h"

namespace HoshioEngine {
	class EditorPanel {
	public:
		EditorPanel() = default;
		virtual ~EditorPanel() = default;
		virtual void Render() = 0;
	};
}



#endif // !_EDITOR_PANEL_H_
