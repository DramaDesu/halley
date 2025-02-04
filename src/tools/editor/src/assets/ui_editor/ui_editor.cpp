#include "ui_editor.h"

#include "ui_editor_display.h"
#include "ui_widget_editor.h"
#include "ui_widget_list.h"
#include "halley/tools/project/project.h"
#include "src/scene/choose_window.h"
#include "src/ui/project_window.h"
using namespace Halley;

UIEditor::UIEditor(UIFactory& factory, Resources& gameResources, Project& project, ProjectWindow& projectWindow, const HalleyAPI& api)
	: AssetEditor(factory, gameResources, project, AssetType::UIDefinition)
	, projectWindow(projectWindow)
{
	project.withDLL([&] (ProjectDLL& dll)
	{
		dll.addReloadListener(*this);
	});
}

UIEditor::~UIEditor()
{
	projectWindow.getProject().withDLL([&](ProjectDLL& dll)
	{
		dll.removeReloadListener(*this);
	});
}

void UIEditor::update(Time time, bool moved)
{
	if (pendingLoad && project.isDLLLoaded()) {
		open();
		pendingLoad = false;
	}
}

void UIEditor::open()
{
	loadGameFactory();

	uiDefinition = std::make_shared<UIDefinition>(*gameResources.get<UIDefinition>(assetId));
	if (widgetList) {
		widgetList->setDefinition(uiDefinition);
	}

	factory.loadUI(*this, "halley/ui_editor");
}

void UIEditor::onMakeUI()
{
	display = getWidgetAs<UIEditorDisplay>("display");
	display->setUIEditor(this);
	widgetList = getWidgetAs<UIWidgetList>("widgetList");
	widgetList->setUIEditor(*this);
	widgetList->setDefinition(uiDefinition);
	widgetEditor = getWidgetAs<UIWidgetEditor>("widgetEditor");
	widgetEditor->setUIEditor(*this, projectWindow);
	widgetEditor->setGameResources(gameResources);

	setHandle(UIEventType::ListSelectionChanged, "widgetsList", [=] (const UIEvent& event)
	{
		setSelectedWidget(event.getStringData());
	});

	setHandle(UIEventType::ButtonClicked, "addWidget", [=] (const UIEvent& event)
	{
		addWidget();
	});

	setHandle(UIEventType::ButtonClicked, "removeWidget", [=] (const UIEvent& event)
	{
		removeWidget();
	});

	doLoadUI();
	reselectWidget();
}

void UIEditor::markModified()
{
	uiDefinition->increaseAssetVersion();
	modified = true;
}

void UIEditor::onWidgetModified(const String& id)
{
	auto data = uiDefinition->findUUID(id);
	if (data.result) {
		widgetList->onWidgetModified(id, *data.result);
	}
	markModified();
}

void UIEditor::reselectWidget()
{
	setSelectedWidget(getWidgetAs<UIList>("widgetsList")->getSelectedOptionId());
}

bool UIEditor::isModified()
{
	return modified;
}

void UIEditor::save()
{
	if (modified) {
		modified = false;

		const auto assetPath = Path("ui/" + uiDefinition->getAssetId() + ".yaml");
		const auto strData = uiDefinition->toYAML();

		project.setAssetSaveNotification(false);
		project.writeAssetToDisk(assetPath, gsl::as_bytes(gsl::span<const char>(strData.c_str(), strData.length())));
		project.setAssetSaveNotification(true);
	}
}

UIFactory& UIEditor::getGameFactory()
{
	return *gameFactory;
}

bool UIEditor::onKeyPress(KeyboardKeyPress key)
{
	if (key.is(KeyCode::Delete)) {
		removeWidget();
	}

	return false;
}

void UIEditor::onProjectDLLStatusChange(ProjectDLL::Status status)
{
	if (status == ProjectDLL::Status::Unloaded) {
		if (display) {
			display->setUIEditor(nullptr);
		}
		gameFactory = {};
		gameI18N = {};
		loaded = false;
	} else if (status == ProjectDLL::Status::Loaded) {
		loadGameFactory();
		if (display) {
			display->setUIEditor(this);
		}
		doLoadUI();
	}
}

void UIEditor::reload()
{
	doLoadUI();
}

std::shared_ptr<const Resource> UIEditor::loadResource(const String& id)
{
	if (project.isDLLLoaded()) {
		open();
		return uiDefinition;
	} else {
		pendingLoad = true;
		return {};
	}
}

void UIEditor::doLoadUI()
{
	if (uiDefinition && display && !loaded) {
		display->loadDisplay(*uiDefinition);
		loaded = true;
	}
}

void UIEditor::setSelectedWidget(const String& id)
{
	curSelection = id;
	const auto result = uiDefinition->findUUID(id);
	widgetEditor->setSelectedWidget(id, result.result, result.parent);
	display->setSelectedWidget(id);
}

void UIEditor::addWidget()
{
	const auto window = std::make_shared<ChooseUIWidgetWindow>(factory, *gameFactory, [=] (std::optional<String> result)
	{
		if (result) {
			addWidget(result.value());
		}
	});
	getRoot()->addChild(window);
}

void UIEditor::addWidget(const String& widgetClass)
{
	ConfigNode data = ConfigNode::MapType();
	if (widgetClass == "sizer") {
		data["sizer"] = ConfigNode::MapType();
	} else if (widgetClass == "spacer") {
		data["spacer"] = ConfigNode::MapType();
	} else {
		ConfigNode::MapType widget;
		widget["class"] = widgetClass;
		data["widget"] = std::move(widget);
	}
	data["uuid"] = UUID::generate().toString();

	addWidget(curSelection, false, std::move(data));
}

void UIEditor::addWidget(const String& referenceId, bool requestedAsChild, ConfigNode data)
{
	auto result = uiDefinition->findUUID(referenceId);
	if (result.result) {
		auto& referenceNode = *result.result;
		const auto widgetClass = referenceNode.hasKey("widget") ? referenceNode["widget"]["class"].asString("") : "sizer";
		const bool canHaveChildren = widgetClass == "sizer" || gameFactory->getPropertiesForWidget(widgetClass).canHaveChildren;
		const bool canHaveSiblings = result.parent != nullptr;
		if (!canHaveChildren && !canHaveSiblings) {
			// Give up
			return;
		}
		const bool asChild = (requestedAsChild && canHaveChildren) || (!canHaveSiblings);

		auto& parent = asChild ? referenceNode : *result.parent;
		parent["children"].ensureType(ConfigNodeType::Sequence);
		auto& parentChildren = parent["children"].asSequence();
		const auto childIdx = std::min(parentChildren.size(), asChild ? std::numeric_limits<size_t>::max() : size_t(result.childIdx + 1));

		widgetList->addWidget(data, parent["uuid"].asString(), childIdx);
		parentChildren.insert(parentChildren.begin() + childIdx, std::move(data));

		markModified();
	}
}

void UIEditor::removeWidget()
{
	removeWidget(curSelection);
}

void UIEditor::removeWidget(const String& id)
{
	auto result = uiDefinition->findUUID(id);
	if (result.result && result.parent) {
		auto& parentChildren = (*result.parent)["children"].asSequence();
		std_ex::erase_if(parentChildren, [=] (const ConfigNode& n) { return &n == result.result; });
		markModified();
		widgetList->getList().removeItem(id);
	}
}

void UIEditor::loadGameFactory()
{
	gameI18N = std::make_unique<I18N>(gameResources, I18NLanguage("en-GB"));
	auto* game = project.getGameInstance();
	gameFactory = game->createUIFactory(projectWindow.getAPI(), gameResources, *gameI18N);
}

ChooseUIWidgetWindow::ChooseUIWidgetWindow(UIFactory& factory, UIFactory& gameFactory, Callback callback)
	: ChooseAssetWindow(Vector2f(), factory, std::move(callback), {})
	, factory(factory)
	, gameFactory(gameFactory)
{
	auto ids = gameFactory.getWidgetClassList();
	ids.push_back("sizer");
	ids.push_back("spacer");

	setAssetIds(std::move(ids), "widget");
	setTitle(LocalisedString::fromHardcodedString("Choose Widget"));
}

std::shared_ptr<UIImage> ChooseUIWidgetWindow::makeIcon(const String& id, bool hasSearch)
{
	String iconName;
	if (id == "sizer") {
		iconName = "widget_icons/sizer_horizontal.png";
	} else if (id == "spacer") {
		iconName = "widget_icons/spacer.png";
	} else {
		iconName = gameFactory.getPropertiesForWidget(id).iconName;
	}
	auto sprite = iconName.isEmpty() ? Sprite() : Sprite().setImage(gameFactory.getResources(), iconName);
	return std::make_shared<UIImage>(std::move(sprite));
}

LocalisedString ChooseUIWidgetWindow::getItemLabel(const String& id, const String& name, bool hasSearch)
{
	String label;
	if (id == "sizer") {
		label = "Sizer";
	} else if (id == "spacer") {
		label = "Spacer";
	} else {
		label = gameFactory.getPropertiesForWidget(id).name;
	}

	return LocalisedString::fromUserString(label);
}

void ChooseUIWidgetWindow::sortItems(Vector<std::pair<String, String>>& values)
{
}

int ChooseUIWidgetWindow::getNumColumns(Vector2f scrollPaneSize) const
{
	return 3;
}
