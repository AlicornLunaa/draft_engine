#include "../cli/commands.hpp"
#include "draft/build_tools/project_manifest.hpp"
#include "draft/util/logger.hpp"

#include <cstdio>
#include <filesystem>
#include <string>

using namespace Draft;
namespace fs = std::filesystem;

int run_manifest(int argc, char** argv) {
    if (argc < 4) {
        std::fprintf(stderr, "Usage: %s manifest <project-root> <module-path-relative-to-root>\n", argv[0]);
        return 1;
    }

    fs::path projectRoot = fs::absolute(argv[2]);
    if (!fs::is_directory(projectRoot)) {
        Logger::println(LogLevel::Critical, "Manifest", "Project root does not exist or is not a directory: " + projectRoot.string());
        return 1;
    }

    try {
        write_module_manifest(projectRoot, argv[3]);
    } catch (const std::exception& e) {
        Logger::println(LogLevel::Critical, "Manifest", std::string("Failed: ") + e.what());
        return 1;
    }

    return 0;
}
