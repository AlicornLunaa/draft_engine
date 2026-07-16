#include "draft/build_tools/project_exporter.hpp"
#include "draft/build_tools/project_packer.hpp"
#include "draft/util/logger.hpp"

#include <cstdlib>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

namespace {
    void run_cmake_step(const std::string& command, const std::string& description) {
        Draft::Logger::println(Draft::LogLevel::Info, "Export", description);
        int result = std::system(command.c_str());
        if (result != 0)
            throw std::runtime_error("Export: " + description + " failed (cmake exited " + std::to_string(result) + ")");
    }

    std::string quote(const fs::path& path) {
        return "\"" + path.string() + "\"";
    }
}

namespace Draft {
    void export_project(const fs::path& projectRoot, const fs::path& outputDir) {
        if (!fs::is_directory(projectRoot))
            throw std::runtime_error("Export: '" + projectRoot.string() + "' is not a directory");

        fs::create_directories(outputDir);

        // Scratch build tree, scoped under outputDir rather than inside projectRoot itself so
        // this never collides with a developer's own (likely Debug) "build/" directory from
        // ordinary day-to-day development.
        fs::path buildDir = outputDir / ".build";

        run_cmake_step(
            "cmake -S " + quote(projectRoot) + " -B " + quote(buildDir) + " -DCMAKE_BUILD_TYPE=Release",
            "Configuring " + projectRoot.string() + " (Release)"
        );
        run_cmake_step(
            "cmake --build " + quote(buildDir) + " -j --config Release",
            "Building"
        );
        // Relies on draft_add_game_module's install(TARGETS ... COMPONENT DraftGameLauncher)
        // rule (engine/build_tools/CMakeLists.txt) to find the launcher binary generically,
        // without needing to know whatever target name the project itself chose.
        run_cmake_step(
            "cmake --install " + quote(buildDir) + " --prefix " + quote(outputDir) + " --component DraftGameLauncher",
            "Installing launcher into " + outputDir.string()
        );

        fs::remove_all(buildDir); // scratch, not part of the exported output

        fs::path apakPath = outputDir / (projectRoot.filename().string() + ".apak");
        Logger::println(LogLevel::Info, "Export", "Packing assets...");
        if (!pack_project(projectRoot, apakPath))
            throw std::runtime_error("Export: asset packing failed, see Pack log above");

        Logger::println(LogLevel::Info, "Export", "Exported to " + outputDir.string());
    }
}
