#pragma once

#include <cstdint>

namespace Halley {
    enum class ScriptNodeExecutionState {
		Done,
		Executing,
		Restart,
		Terminate,
    	Merged
	};

	enum class ScriptNodeClassification {
		FlowControl,
		Action,
		Variable,
		Terminator, // As in start/end, not as in Arnie
		Unknown
	};

	enum class ScriptNodeElementType : uint8_t {
		Undefined,
		Node,
		FlowPin,
		ReadDataPin,
		WriteDataPin,
		TargetPin
	};

	enum class ScriptNodePinDirection : uint8_t {
		Input,
		Output
	};

	enum class ScriptPinSide : uint8_t {
		Undefined,
		Left,
		Right,
		Top,
		Bottom
	};

	struct ScriptNodePinType {
		ScriptNodeElementType type = ScriptNodeElementType::Undefined;
		ScriptNodePinDirection direction = ScriptNodePinDirection::Input;

		bool operator==(const ScriptNodePinType& other) const
		{
			return type == other.type && direction == other.direction;
		}
		bool operator!=(const ScriptNodePinType& other) const
		{
			return type != other.type || direction != other.direction;
		}

		ScriptPinSide getSide() const
		{
			switch (type) {
			case ScriptNodeElementType::ReadDataPin:
			case ScriptNodeElementType::WriteDataPin:
			case ScriptNodeElementType::FlowPin:
				return direction == ScriptNodePinDirection::Input ? ScriptPinSide::Left : ScriptPinSide::Right;
			case ScriptNodeElementType::TargetPin:
				return direction == ScriptNodePinDirection::Input ? ScriptPinSide::Top : ScriptPinSide::Bottom;
			default:
				return ScriptPinSide::Undefined;
			}
		}

		bool isMultiConnection() const
		{
			return (type == ScriptNodeElementType::ReadDataPin && direction == ScriptNodePinDirection::Output)
				|| (type == ScriptNodeElementType::WriteDataPin && direction == ScriptNodePinDirection::Input)
				|| (type == ScriptNodeElementType::FlowPin)
				|| (type == ScriptNodeElementType::TargetPin && direction == ScriptNodePinDirection::Output);
		}
	};
}
