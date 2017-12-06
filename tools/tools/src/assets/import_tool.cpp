#include "halley/tools/assets/import_tool.h"
#include "halley/tools/codegen/codegen.h"
#include <iostream>
#include "halley/tools/project/project.h"
#include "halley/tools/tasks/editor_task_set.h"
#include "halley/tools/assets/check_assets_task.h"
#include <thread>
#include <chrono>
#include "halley/tools/file/filesystem.h"
#include "halley/os/os.h"
#include "halley/core/game/halley_statics.h"
#include "halley/support/logger.h"

using namespace Halley;
using namespace std::chrono_literals;

int ImportTool::run(Vector<std::string> args)
{
	if (args.size() >= 2) {
		HalleyStatics statics;
		statics.resume(nullptr);
		StdOutSink logSink;
		Logger::addSink(logSink);

		Path projectPath = FileSystem::getAbsolute(Path(args[0]));
		Path halleyRootPath = FileSystem::getAbsolute(Path(args[1]));

		String platform = "pc";

		for (size_t i = 2; i < args.size(); ++i) {
			String arg = args[i];
			if (arg.startsWith("--platform=")) {
				platform = arg.mid(11);
			}
		}

		auto proj = std::make_unique<Project>(statics, platform, projectPath, halleyRootPath);
		std::cout << "Importing project at " << projectPath << ", with Halley root at " << halleyRootPath << "" << std::endl;

		auto tasks = std::make_unique<EditorTaskSet>();
		tasks->setListener(*this);
		tasks->addTask(EditorTaskAnchor(std::make_unique<CheckAssetsTask>(*proj, true)));
		auto last = std::chrono::steady_clock::now();

		while (tasks->getTasks().size() > 0) {
			std::this_thread::sleep_for(50ms);

			auto now = std::chrono::steady_clock::now();
			float elapsed = std::chrono::duration<float>(now - last).count();
			last = now;

			tasks->update(elapsed);
		}

		if (hasError) {
			std::cout << "Import failed." << std::endl;
			return 1;
		} else {
			std::cout << "Import done." << std::endl;
			return 0;
		}
	}
	else {
		std::cout << "Usage: halley-cmd import projDir halleyDir" << std::endl;
		return 1;
	}
}

void ImportTool::onTaskAdded(const std::shared_ptr<EditorTaskAnchor>&)
{
}

void ImportTool::onTaskTerminated(const std::shared_ptr<EditorTaskAnchor>&)
{
}

void ImportTool::onTaskError(const std::shared_ptr<EditorTaskAnchor>&)
{
	hasError = true;
}
