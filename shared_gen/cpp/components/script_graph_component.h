// Halley codegen version 105
#pragma once

#ifndef DONT_INCLUDE_HALLEY_HPP
#include <halley.hpp>
#endif

class ScriptGraphComponent final : public Halley::Component {
public:
	static constexpr int componentIndex{ 9 };
	static const constexpr char* componentName{ "ScriptGraph" };

	Halley::ScriptGraph scriptGraph{};

	ScriptGraphComponent() {
	}

	ScriptGraphComponent(Halley::ScriptGraph scriptGraph)
		: scriptGraph(std::move(scriptGraph))
	{
	}

	Halley::ConfigNode serialize(const Halley::EntitySerializationContext& context) const {
		using namespace Halley::EntitySerialization;
		Halley::ConfigNode node = Halley::ConfigNode::MapType();
		Halley::EntityConfigNodeSerializer<decltype(scriptGraph)>::serialize(scriptGraph, Halley::ScriptGraph{}, context, node, componentName, "scriptGraph", makeMask(Type::Prefab));
		return node;
	}

	void deserialize(const Halley::EntitySerializationContext& context, const Halley::ConfigNode& node) {
		using namespace Halley::EntitySerialization;
		Halley::EntityConfigNodeSerializer<decltype(scriptGraph)>::deserialize(scriptGraph, Halley::ScriptGraph{}, context, node, componentName, "scriptGraph", makeMask(Type::Prefab));
	}

};
