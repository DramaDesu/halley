#include "halley/core/game/game.h"

#include "halley/editor_extensions/asset_preview_generator.h"
#include "halley/entity/scripting/script_node_type.h"
#include "halley/ui/ui_factory.h"
using namespace Halley;

Game::~Game() = default;

void Game::init(const Environment&, const Vector<String>&)
{}

ResourceOptions Game::initResourceLocator(const Path& gamePath, const Path& assetsPath, const Path& unpackedAssetsPath, ResourceLocator& locator)
{
	return {};
}

bool Game::shouldCreateSeparateConsole() const
{
	return isDevMode();
}

Game::ConsoleInfo Game::getConsoleInfo() const
{
	return ConsoleInfo{ getName() + " [Console]", {}, Vector2f(0.5f, 0.5f) };
}

void Game::endGame()
{}

std::unique_ptr<Stage> Game::makeStage(StageID)
{
	return {};
}

std::unique_ptr<IFrameData> Game::makeFrameData()
{
	return std::make_unique<EmptyFrameData>();
}

double Game::getTargetFPS() const
{
	return 0.0;
}

double Game::getTargetBackgroundFPS() const
{
	return getTargetFPS();
}

double Game::getFixedUpdateFPS() const
{
	return 60.0;
}

String Game::getDevConAddress() const
{
	return "";
}

int Game::getDevConPort() const
{
	return 12500;
}

std::shared_ptr<GameConsole> Game::getGameConsole() const
{
	return {};
}

void Game::onUncaughtException(const Exception& exception, TimeLine timeLine)
{
	throw exception;
}

std::unique_ptr<ISceneEditor> Game::createSceneEditorInterface()
{
	return {};
}

std::unique_ptr<IEditorCustomTools> Game::createEditorCustomToolsInterface()
{
	return {};
}

std::unique_ptr<AssetPreviewGenerator> Game::createAssetPreviewGenerator(const HalleyAPI& api, Resources& resources)
{
	return std::make_unique<AssetPreviewGenerator>(*this, api, resources);
}

std::unique_ptr<UIFactory> Game::createUIFactory(const HalleyAPI& api, Resources& resources, I18N& i18n)
{
	auto factory = std::make_unique<UIFactory>(api, resources, i18n);
	factory->loadStyleSheetsFromResources();
	return factory;
}

std::unique_ptr<ScriptNodeTypeCollection> Game::createScriptNodeTypeCollection()
{
	return std::make_unique<ScriptNodeTypeCollection>();
}

Vector<std::unique_ptr<IComponentEditorFieldFactory>> Game::createCustomEditorFieldFactories(Resources& gameResources)
{
	return {};
}

void Game::attachToEditorDebugConsole(UIDebugConsoleCommands& commands, Resources& gameResources, IProject& project)
{
}

const HalleyAPI& Game::getAPI() const
{
	if (!api) {
		throw Exception("HalleyAPI is only initialized on Game right before call to startGame()", HalleyExceptions::Core);
	}
	return *api;
}

Resources& Game::getResources() const
{
	if (!resources) {
		throw Exception("Resources are only initialized on Game right before call to startGame()", HalleyExceptions::Core);
	}
	return *resources;
}

size_t Game::getMaxThreads() const
{
	return std::thread::hardware_concurrency();
}
