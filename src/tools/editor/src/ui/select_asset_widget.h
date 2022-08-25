#pragma once
#include "halley/ui/ui_widget.h"

namespace Halley {
	class ProjectWindow;
	class UIFactory;
	class UITextInput;

	class SelectTargetWidget : public UIWidget {
	public:
		SelectTargetWidget(const String& id, UIFactory& factory, IProjectWindow& projectWindow);
		virtual ~SelectTargetWidget() override;

		void setValue(const String& newValue);
		String getValue() const;

		void setDefaultAssetId(String assetId);
		void setAllowEmpty(std::optional<String> allowEmpty);
		void setDisplayErrorForEmpty(bool enabled);

	protected:
		void makeUI();

		virtual std::shared_ptr<UIWidget> makeChooseWindow(std::function<void(std::optional<String>)> callback) = 0;
		virtual void goToValue(KeyMods keyMods);
		virtual bool hasGoTo() const;
		virtual bool valueExists(const String& value);
		virtual Sprite makeIcon();
		virtual String doGetDisplayName(const String& name) const;

		UIFactory& factory;
		ProjectWindow& projectWindow;
		String value;
		String defaultAssetId;
		std::optional<String> allowEmpty;

	private:
		void choose();
		void updateToolTip();

		void readFromDataBind() override;
		String getDisplayName() const;
		String getDisplayName(const String& name) const;

		std::shared_ptr<UITextInput> input;
		std::shared_ptr<bool> aliveFlag;
		bool displayErrorForEmpty = true;
		bool firstValue = true;
	};

	class SelectAssetWidget : public SelectTargetWidget {
	public:
		SelectAssetWidget(const String& id, UIFactory& factory, AssetType type, Resources& gameResources, IProjectWindow& projectWindow);

	protected:
		std::shared_ptr<UIWidget> makeChooseWindow(std::function<void(std::optional<String>)> callback) override;
		void goToValue(KeyMods keyMods) override;
		bool hasGoTo() const override;
		bool valueExists(const String& value) override;
		Sprite makeIcon() override;
		String doGetDisplayName(const String& name) const override;

	private:
		AssetType type;
		Resources& gameResources;
	};
}
