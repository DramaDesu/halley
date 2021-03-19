#pragma once
#include "halley/core/graphics/sprite/animation.h"
#include "halley/editor_extensions/scene_editor_gizmo.h"

namespace Halley {
	class Material;
	
	class SelectedBoundsGizmo final : public SceneEditorGizmo {
	public:
		SelectedBoundsGizmo(SnapRules snapRules, Resources& resources);

		void update(Time time, const ISceneEditor& sceneEditor, const SceneEditorInputState& inputState) override;
		void draw(Painter& painter) const override;

	private:
		std::optional<Rect4f> bounds;
		std::shared_ptr<Material> material;
	};
}
