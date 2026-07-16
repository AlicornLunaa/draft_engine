#define GLFW_INCLUDE_NONE

#include "../cli/commands.hpp"
#include "draft/asset/asset_manager.hpp"
#include "draft/build_tools/asset_pipeline.hpp"
#include "draft/core/engine.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/logger.hpp"

#include "GLFW/glfw3.h"

#include <cstdio>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

using namespace Draft;
namespace fs = std::filesystem;

int run_validate(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "Usage: %s validate <project-root> [asset-path]\n", argv[0]);
        return 1;
    }

    fs::path projectRoot = fs::absolute(argv[2]);
    if (!fs::is_directory(projectRoot)) {
        Logger::println(LogLevel::Critical, "Validate", "Project root does not exist or is not a directory: " + projectRoot.string());
        return 1;
    }

    std::vector<AssetTask> tasks;

    if (argc >= 4) {
        fs::path assetPath = fs::absolute(argv[3]);
        if (!fs::is_regular_file(assetPath)) {
            Logger::println(LogLevel::Critical, "Validate", "Asset does not exist or is not a regular file: " + assetPath.string());
            return 1;
        }

        fs::path relative = fs::relative(assetPath, projectRoot);
        AssetKind kind = classify_asset(relative);
        if (kind == AssetKind::Unknown) {
            Logger::println(LogLevel::Critical, "Validate", "No validator registered for extension \"" + relative.extension().string() + "\"");
            return 1;
        }

        tasks.push_back({relative.generic_string(), kind});
    } else {
        if (!fs::is_directory(projectRoot / "assets")) {
            Logger::println(LogLevel::Critical, "Validate", "No assets/ directory under " + projectRoot.string());
            return 1;
        }

        tasks = collect_project_assets(projectRoot);
    }

    // Texture/Font/Model construction all issue real GL calls (Font bakes glyphs, Model uploads
    // meshes/embedded textures), so a live GL context has to exist for the whole run
    glfwInit();
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    RenderWindow window(64, 64, "draft_buildtools_validate");

    // AssetManager resolves queue()/get() keys relative to the current working directory (via
    // DiskFileProvider), the same convention test_bench's "assets/..." keys already rely on.
    fs::current_path(projectRoot);

    AssetManager assets;
    Engine sceneEngine;
    std::unordered_map<std::string, std::string> errors = validate_assets(assets, sceneEngine, tasks);

    for (const AssetTask& task : tasks) {
        auto it = errors.find(task.key);
        if (it == errors.end()) {
            Logger::println(LogLevel::Info, asset_kind_name(task.kind), task.key + " - OK");
        } else {
            Logger::println(LogLevel::Severe, asset_kind_name(task.kind), task.key + " - FAILED: " + it->second);
        }
    }

    std::size_t failed = errors.size();
    Logger::println(failed == 0 ? LogLevel::Info : LogLevel::Critical, "Validate",
        std::to_string(tasks.size() - failed) + "/" + std::to_string(tasks.size()) + " assets valid");

    return failed == 0 ? 0 : 1;
}
