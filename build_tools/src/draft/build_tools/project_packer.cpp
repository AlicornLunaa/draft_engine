#define GLFW_INCLUDE_NONE

#include "draft/build_tools/project_packer.hpp"
#include "draft/asset/asset_manager.hpp"
#include "draft/build_tools/apak_writer.hpp"
#include "draft/build_tools/asset_pipeline.hpp"
#include "draft/core/engine.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/json.hpp"
#include "draft/util/logger.hpp"

#include "GLFW/glfw3.h"

#include <chrono>
#include <string>

namespace fs = std::filesystem;

namespace {
    // Restores the process's current_path() on scope exit, success or not
    struct CwdGuard {
        fs::path previous = fs::current_path();
        ~CwdGuard() { fs::current_path(previous); }
    };

    // Copies one validated asset into tempDir, source-relative path preserved. Scene JSON is
    // re-parsed and re-dumped (consistent key order/formatting, "the packed copy is
    // normalized"); everything else is opaque binary data, copied byte for byte with no
    // decode/re-encode.
    void resave(const Draft::AssetTask& task, const fs::path& tempDir) {
        Draft::HostFileSystem hostFs;
        fs::path destination = tempDir / task.key;

        if (task.kind == Draft::AssetKind::Scene) {
            Draft::JSON parsed = Draft::JSON::parse(hostFs.read_string(task.key));
            hostFs.write_string(destination, parsed.dump(2));
        } else {
            hostFs.copy(task.key, destination);
        }
    }
}

namespace Draft {
    bool pack_project(const fs::path& projectRoot, const fs::path& outputPath) {
        CwdGuard cwdGuard;

        // Texture/Font/Model construction all issue real GL calls, same as validate.
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        RenderWindow window(64, 64, "draft_buildtools_pack");

        fs::current_path(projectRoot);

        std::vector<AssetTask> tasks = collect_project_assets(projectRoot);

        AssetManager assets;
        Engine sceneEngine;

        // Validate first and abort on any failure
        auto errors = validate_assets(assets, sceneEngine, tasks);
        if (!errors.empty()) {
            for (const auto& [key, message] : errors)
                Logger::println(LogLevel::Severe, "Pack", key + " - FAILED: " + message);

            Logger::println(LogLevel::Critical, "Pack",
                "Aborting: " + std::to_string(errors.size()) + "/" + std::to_string(tasks.size()) + " assets failed validation");
            return false;
        }

        Logger::println(LogLevel::Info, "Pack", std::to_string(tasks.size()) + "/" + std::to_string(tasks.size()) + " assets valid, resaving...");

        fs::path tempDir = fs::temp_directory_path() / ("draft_pack_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(tempDir);

        try {
            for (const AssetTask& task : tasks) {
                resave(task, tempDir);
                Logger::println(LogLevel::Info, "Pack", task.key + " - resaved");
            }

            ApakWriter::write(tempDir, outputPath);
        } catch (...) {
            fs::remove_all(tempDir);
            throw;
        }

        fs::remove_all(tempDir);
        Logger::println(LogLevel::Info, "Pack", "Wrote " + outputPath.string());
        return true;
    }
}
