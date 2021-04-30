#pragma once
#include <optional>
#include "halley/core/game/scene_editor_interface.h"
#include "halley/entity/entity.h"
#include "halley/maths/line.h"
#include "halley/time/halleytime.h"

namespace Halley {
	struct SceneEditorOutputState;
	struct SceneEditorInputState;
	class Camera;
	class Painter;
	class UIWidget;

	enum class GridSnapMode : uint8_t {
		Disabled,
		Pixel
	};
	
	enum class LineSnapMode : uint8_t {
		Disabled,
		AxisAligned,
		IsometricAxisAligned
	};

	struct SnapRules {
		GridSnapMode grid = GridSnapMode::Disabled;
		LineSnapMode line = LineSnapMode::Disabled;
	};

	class SceneEditorGizmoHandle {
	public:
		using SnapFunction = std::function<Vector2f(int, Vector2f)>;
		using BoundsCheckFunction = std::function<bool(Vector2f, Vector2f)>;

		SceneEditorGizmoHandle(String id = "");
		void update(const SceneEditorInputState& inputState, gsl::span<SceneEditorGizmoHandle> handles = {});

		void setIndex(int index);
		int getIndex() const;

		const String& getId() const { return id; }
		
		void setPosition(Vector2f pos, bool snap);
		Vector2f getPosition() const;
		
		bool isOver() const;
		bool isHeld() const;
		bool isSelected() const;
		bool isEnabled() const;

		void setCanDrag(bool enabled);
		void setNotOver();
		void setSelected(bool sel);

		void setBoundsCheck(BoundsCheckFunction boundsCheck);
		void setSnap(SnapFunction snapFunc);
		void setGridSnap(GridSnapMode gridSnap);
		void setEnabled(bool enabled);

	private:
		int index = 0;
		bool over = false;
		bool holding = false;
		bool canDrag = true;
		bool selected = false;
		bool enabled = true;

		Vector2f pos;
		Vector2f startOffset;
		String id;

		BoundsCheckFunction boundsCheck;
		SnapFunction snapFunc;
	};

	class SceneEditorGizmo {
	public:
		explicit SceneEditorGizmo(SnapRules snapRules);
		virtual ~SceneEditorGizmo() = default;

		virtual void update(Time time, const ISceneEditor& sceneEditor, const SceneEditorInputState& inputState);
		virtual void draw(Painter& painter) const;
		virtual std::shared_ptr<UIWidget> makeUI();

		void setSelectedEntity(const std::optional<EntityRef>& entity, EntityData& entityData);
		virtual void refreshEntity();

		void setCamera(const Camera& camera);
		void setOutputState(SceneEditorOutputState* outputState);

		virtual bool isHighlighted() const;
		virtual void deselect();
		
		virtual std::vector<String> getHighlightedComponents() const;

	protected:
		virtual void onEntityChanged();

		const std::optional<EntityRef>& getEntity() const;
		std::optional<EntityRef>& getEntity();

		EntityData& getEntityData();
		bool hasEntityData() const;
		ConfigNode* getComponentData(const String& name);
		const ConfigNode* getComponentData(const String& name) const;

		void markModified(const String& component, const String& field);
		
		float getZoom() const;
		SnapRules getSnapRules() const;
		Vector2f solveLineSnap(Vector2f cur, std::optional<Vector2f> prev, std::optional<Vector2f> next) const;

		template <typename T>
		T* getComponent()
		{
			if (curEntity) {
				return curEntity->tryGetComponent<T>();
			} else {
				return nullptr;
			}
		}
		
		template <typename T>
		const T* getComponent() const
		{
			if (curEntity) {
				return curEntity->tryGetComponent<T>();
			} else {
				return nullptr;
			}
		}

	private:
		std::optional<EntityRef> curEntity;
		EntityData* entityData = nullptr;
		SceneEditorOutputState* outputState = nullptr;
		float zoom = 1.0f;

		SnapRules snapRules;

		std::optional<Line> findSnapLine(Vector2f cur, Vector2f ref) const;
	};
}
