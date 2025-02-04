#include "script_input.h"

#include "halley/core/input/input_device.h"
#include "halley/support/logger.h"
using namespace Halley;

ScriptInputButtonData::ScriptInputButtonData(const ScriptInputButtonData& other)
{
	*this = other;
}

ScriptInputButtonData& ScriptInputButtonData::operator=(const ScriptInputButtonData& other)
{
	outputMask = other.outputMask;
	// Don't copy input
	return *this;
}

ConfigNode ScriptInputButtonData::toConfigNode(const EntitySerializationContext& context)
{
	return ConfigNode(outputMask);
}

void ScriptInputButtonData::finishData()
{
	outputMask = 0;
}

String ScriptInputButton::getLabel(const ScriptGraphNode& node) const
{
	return node.getSettings()["button"].asString("");
}

Vector<IScriptNodeType::SettingType> ScriptInputButton::getSettingTypes() const
{
	return {
		SettingType{ "button", "Halley::InputButton", Vector<String>{""} },
		SettingType{ "priority", "Halley::InputPriority", Vector<String>{"normal"} },
		SettingType{ "label", "Halley::String", Vector<String>{""} },
		SettingType{ "bypassEnableCheck", "bool", Vector<String>{""} },
	};
}

gsl::span<const IScriptNodeType::PinType> ScriptInputButton::getPinConfiguration(const ScriptGraphNode& node) const
{
	using ET = ScriptNodeElementType;
	using PD = GraphNodePinDirection;
	const static auto data = std::array<PinType, 8>{ PinType{ ET::FlowPin, PD::Input }, PinType{ ET::TargetPin, PD::Input }, PinType{ ET::FlowPin, PD::Output },
		PinType{ ET::FlowPin, PD::Output }, PinType{ ET::FlowPin, PD::Output, true }, PinType{ ET::FlowPin, PD::Output, true },
		PinType{ ET::FlowPin, PD::Output, true }, PinType{ ET::ReadDataPin, PD::Input } };
	return data;
}

std::pair<String, Vector<ColourOverride>> ScriptInputButton::getNodeDescription(const ScriptGraphNode& node, const World* world, const ScriptGraph& graph) const
{
	auto str = ColourStringBuilder(true);
	str.append("Input ");
	str.append(node.getSettings()["button"].asString(""), settingColour);
	str.append(" on ");
	str.append(getConnectedNodeName(world, node, graph, 1), parameterColour);
	str.append(" with priority ");
	str.append(node.getSettings()["priority"].asString("normal"), parameterColour);
	if (node.getPin(7).hasConnection()) {
		str.append(" and label ");
		str.append(getConnectedNodeName(world, node, graph, 7), parameterColour);
	}
	else {
		if (node.getSettings()["label"].asString("") != "") {
			str.append(" and label ");
			str.append(node.getSettings()["label"].asString(""), settingColour);
		}
	}

	if (node.getSettings()["bypassEnableCheck"].asBool(false)) {
		str.append(" and ");
		str.append("bypass enable check", settingColour);
	}
	return str.moveResults();
}

String ScriptInputButton::getPinDescription(const ScriptGraphNode& node, PinType element, GraphPinId elementIdx) const
{
	switch (elementIdx) {
	case 2:
		return "Flow Output when button is pressed";
	case 3:
		return "Flow Output when button is released";
	case 4:
		return "Flow Output while button is held";
	case 5:
		return "Flow Output while button is not held";
	case 6:
		return "Flow Output while input is active";
	case 7:
		return "Label";
	default:
		return ScriptNodeTypeBase<ScriptInputButtonData>::getPinDescription(node, element, elementIdx);
	}
}

void ScriptInputButton::doInitData(ScriptInputButtonData& data, const ScriptGraphNode& node, const EntitySerializationContext& context,	const ConfigNode& nodeData) const
{
	data.outputMask = static_cast<uint8_t>(nodeData.asInt(0));
}

IScriptNodeType::Result ScriptInputButton::doUpdate(ScriptEnvironment& environment, Time time, const ScriptGraphNode& node, ScriptInputButtonData& data) const
{
	constexpr uint8_t pressedPin = 1;
	constexpr uint8_t releasedPin = 2;
	constexpr uint8_t heldPin = 4;
	constexpr uint8_t notHeldPin = 8;
	constexpr uint8_t activePin = 16;

	if (!data.input) {
		const auto entity = readEntityId(environment, node, 1);
		const int button = environment.getInputButtonByName(node.getSettings()["button"].asString("primary"));
		const auto inputDevice = environment.getInputDevice(entity, node.getSettings()["bypassEnableCheck"].asBool(false));
		if (inputDevice) {
			const auto priority = fromString<InputPriority>(node.getSettings()["priority"].asString("normal"));
			auto label = readDataPin(environment, node, 7).asString("");
			if (label.isEmpty()) {
				label = node.getSettings()["label"].asString("");
			}
			data.input = inputDevice->makeExclusiveButton(button, priority, label);
		}
	}

	if (data.input) {
		const auto prevMask = data.outputMask;

		const bool canRead = environment.isInputEnabled() || node.getSettings()["bypassEnableCheck"].asBool(false);
		const bool pressed = canRead && data.input->isPressed();
		const bool released = canRead && data.input->isReleased();
		const bool held = canRead && data.input->isDown();
		const bool isActive = data.input->isActive();

		const uint8_t curMask = (pressed ? pressedPin : 0) | (released ? releasedPin : 0) | (held ? heldPin : notHeldPin) | (isActive ? activePin : 0);
		const uint8_t activate = curMask & ~prevMask;
		const uint8_t cancel = (prevMask & ~curMask) & (heldPin | notHeldPin | activePin);
		data.outputMask = curMask;

		if (activate != 0 || cancel != 0) {
			return Result(ScriptNodeExecutionState::Fork, time, activate, cancel);
		}
	}

	return Result(ScriptNodeExecutionState::Executing, time);
}

void ScriptInputButton::doDestructor(ScriptEnvironment& environment, const ScriptGraphNode& node, ScriptInputButtonData& data) const
{
	data.input.reset();
}
