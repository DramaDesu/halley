#pragma once

#include "halley/core/graphics/sprite/sprite.h"
#include "halley/core/graphics/text/text_renderer.h"
#include <unordered_map>
#include <map>

namespace Halley {
	class ConfigFile;
	class ConfigNode;
	class ConfigObserver;
	class AudioClip;
	class UISTyle;

	class UIStyleDefinition
	{
	public:
		UIStyleDefinition(String styleName, const ConfigNode& node, Resources& resources);
		~UIStyleDefinition();

		const Sprite& getSprite(const String& name) const;
		const TextRenderer& getTextRenderer(const String& name) const;
		Vector4f getBorder(const String& name) const;
		const String& getString(const String& name) const;
		float getFloat(const String& name) const;
		Colour4f getColour(const String& name) const;
		std::shared_ptr<const UIStyleDefinition> getSubStyle(const String& name) const;

		bool hasTextRenderer(const String& name) const;
		bool hasColour(const String& name) const;
		bool hasSubStyle(const String& name) const;

		void reload(const ConfigNode& node);

	private:
		class Pimpl;
		
		const String styleName;
		const ConfigNode* node = nullptr;
		Resources& resources;

		std::unique_ptr<Pimpl> pimpl;
	};

	class UIStyleSheet {
		friend class UIStyle;

	public:
		UIStyleSheet(Resources& resources);
		UIStyleSheet(Resources& resources, const ConfigFile& file);

		void load(const ConfigFile& file);

		bool updateIfNeeded();

	private:
		Resources& resources;
		std::unordered_map<String, std::shared_ptr<UIStyleDefinition>> styles;
		std::map<String, ConfigObserver> observers;

		void load(const ConfigNode& node);
		std::shared_ptr<const UIStyleDefinition> getStyle(const String& styleName) const;

		bool needsUpdate() const;
		void update();
	};
}
