#pragma once
#include "script_node_type.h"
#include "script_node_enums.h"
#include "halley/core/graph/base_graph_renderer.h"
#include "halley/core/graphics/sprite/sprite.h"
#include "halley/maths/bezier.h"
#include "halley/maths/circle.h"
#include "halley/maths/vector2.h"

namespace Halley {
	class IScriptNodeType;
	class ScriptNodeTypeCollection;
	class World;
	class ScriptGraphNode;
	class ScriptGraph;
	class ScriptState;

	class ScriptRenderer : public BaseGraphRenderer {
	public:
		ScriptRenderer(Resources& resources, const World* world, const ScriptNodeTypeCollection& nodeTypeCollection, float nativeZoom);
		
		void setGraph(const BaseGraph* graph);
		void setState(const ScriptState* scriptState);
		void draw(Painter& painter, Vector2f basePos, float curZoom, float posScale = 1.0f);

		std::optional<NodeUnderMouseInfo> getNodeUnderMouse(Vector2f basePos, float curZoom, Vector2f mousePos, bool pinPriority) const;
		Vector2f getPinPosition(Vector2f basePos, const BaseGraphNode& node, GraphPinId idx, float zoom) const;
		Vector<GraphNodeId> getNodesInRect(Vector2f basePos, float curZoom, Rect4f selBox) const;
		void setHighlight(std::optional<NodeUnderMouseInfo> highlightNode, OptionalLite<uint8_t> highlightEntity);
		void setSelection(Vector<GraphNodeId> selectedNodes);
		void setCurrentPaths(Vector<ConnectionPath> path);

		static Colour4f getNodeColour(const IScriptNodeType& nodeType);

	private:
		enum class NodeDrawModeType : uint8_t {
			Normal,
			Highlight,
			Unvisited,
			Active
		};

		struct NodeDrawMode {
			NodeDrawModeType type = NodeDrawModeType::Normal;
			bool selected = false;
			float time = 0;
			float activationTime = std::numeric_limits<float>::infinity();
		};
		
		Resources& resources;
		const World* world = nullptr;
		const ScriptNodeTypeCollection& nodeTypeCollection;
		float nativeZoom = 1.0f;
		
		const ScriptGraph* graph = nullptr;
		const ScriptState* state = nullptr;

		Sprite nodeBg;
		Sprite nodeBgOutline;
		Sprite pinSprite;
		Sprite destructorBg;
		Sprite destructorIcon;
		TextRenderer labelText;
		std::map<String, Sprite> icons;

		void drawNodeOutputs(Painter& painter, Vector2f basePos, GraphNodeId nodeIdx, const ScriptGraph& graph, float curZoom, float posScale);
		void drawNode(Painter& painter, Vector2f basePos, const ScriptGraphNode& node, float curZoom, float posScale, NodeDrawMode drawMode, std::optional<GraphNodePinType> highlightElement, GraphPinId highlightElementId);

		Vector2f getNodeSize(const IScriptNodeType& nodeType, const BaseGraphNode& node, float curZoom) const;
		Vector2f getCommentNodeSize(const BaseGraphNode& node, float curZoom) const;
		Circle getNodeElementArea(const IScriptNodeType& nodeType, Vector2f basePos, const ScriptGraphNode& node, size_t pinN, float curZoom, float posScale) const;
		Colour4f getPinColour(GraphNodePinType pinType) const;
		const Sprite& getIcon(const IScriptNodeType& nodeType, const ScriptGraphNode& node);

		BezierCubic makeBezier(const ConnectionPath& path) const;
		void drawConnection(Painter& painter, const ConnectionPath& path, float curZoom, bool highlight, bool fade) const;

		NodeDrawMode getNodeDrawMode(GraphNodeId nodeId) const;

		GraphPinSide getSide(GraphNodePinType pinType) const;
	};
}
