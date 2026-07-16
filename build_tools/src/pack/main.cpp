#include "../cli/commands.hpp"
#include "draft/build_tools/project_packer.hpp"
#include "draft/util/logger.hpp"

#include <cstdio>
#include <filesystem>
#include <string>

using namespace Draft;
namespace fs = std::filesystem;

int run_pack(int argc, char** argv) {
    if (argc < 4) {
        std::fprintf(stderr, "Usage: %s pack <project-root> <output.apak>\n", argv[0]);
        return 1;
    }

    fs::path projectRoot = fs::absolute(argv[2]);
    if (!fs::is_directory(projectRoot)) {
        Logger::println(LogLevel::Critical, "Pack", "Project root does not exist or is not a directory: " + projectRoot.string());
        return 1;
    }

    if (!fs::is_directory(projectRoot / "assets")) {
        Logger::println(LogLevel::Critical, "Pack", "No assets/ directory under " + projectRoot.string());
        return 1;
    }

    fs::path outputPath = fs::absolute(argv[3]);

    try {
        return pack_project(projectRoot, outputPath) ? 0 : 1;
    } catch (const std::exception& e) {
        Logger::println(LogLevel::Critical, "Pack", std::string("Failed: ") + e.what());
        return 1;
    }
}
