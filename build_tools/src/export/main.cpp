#include "../cli/commands.hpp"
#include "draft/build_tools/project_exporter.hpp"
#include "draft/util/logger.hpp"

#include <cstdio>
#include <filesystem>
#include <string>

using namespace Draft;
namespace fs = std::filesystem;

int run_export(int argc, char** argv) {
    if (argc < 4) {
        std::fprintf(stderr, "Usage: %s export <project-root> <output-dir>\n", argv[0]);
        return 1;
    }

    fs::path projectRoot = fs::absolute(argv[2]);
    fs::path outputDir = fs::absolute(argv[3]);

    try {
        export_project(projectRoot, outputDir);
    } catch (const std::exception& e) {
        Logger::println(LogLevel::Critical, "Export", std::string("Failed: ") + e.what());
        return 1;
    }

    return 0;
}
