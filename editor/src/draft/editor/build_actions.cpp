#define GLFW_INCLUDE_NONE

#include "draft/editor/build_actions.hpp"
#include "draft/asset/asset_manager.hpp"
#include "draft/build_tools/asset_pipeline.hpp"
#include "draft/build_tools/project_exporter.hpp"
#include "draft/build_tools/project_packer.hpp"
#include "draft/core/engine.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/logger.hpp"

#include "GLFW/glfw3.h"

#include <cstdlib>
#include <stdexcept>

namespace fs = std::filesystem;

namespace Draft {
    namespace {
        // Restores the process's current_path() on scope exit, mirroring project_packer.cpp's
        // own CwdGuard (private to that translation unit, so duplicated here rather than shared).
        struct CwdGuard {
            fs::path previous = fs::current_path();
            ~CwdGuard() { fs::current_path(previous); }
        };

        void run_cmake_step(const std::string& command, const std::string& description){
            Logger::println(LogLevel::Info, "Build", description);
            int result = std::system(command.c_str());
            if(result != 0)
                throw std::runtime_error("Build: " + description + " failed (cmake exited " + std::to_string(result) + ")");
        }

        void run_cmake_build(const fs::path& projectRoot){
            fs::path buildDir = projectRoot / "build";

            if(!fs::exists(buildDir / "CMakeCache.txt")){
                run_cmake_step(
                    "cmake -S \"" + projectRoot.string() + "\" -B \"" + buildDir.string() + "\" -DCMAKE_BUILD_TYPE=Debug",
                    "Configuring " + projectRoot.string() + " (Debug)"
                );
            }

            run_cmake_step("cmake --build \"" + buildDir.string() + "\" -j", "Building " + projectRoot.string());
        }
    }

    BuildActions::~BuildActions(){
        if(m_thread)
            m_thread->join();
    }

    std::string BuildActions::label() const {
        switch(m_kind){
            case BuildActionKind::Build: return "Building...";
            case BuildActionKind::ValidateAssets: return "Validating assets...";
            case BuildActionKind::Pack: return "Packing...";
            case BuildActionKind::Export: return "Exporting...";
        }

        return "";
    }

    void BuildActions::start(BuildActionKind kind, std::function<void()> work){
        if(running()){
            Logger::println(LogLevel::Warning, "Editor", "A build action is already running, ignoring request.");
            return;
        }

        m_kind = kind;
        m_finished = false;
        m_succeeded = false;

        m_thread.emplace([this, work = std::move(work)]{
            bool ok = false;

            try {
                work();
                ok = true;
            } catch(const std::exception& e){
                Logger::println(LogLevel::Severe, "Editor", e.what());
            } catch(...){
                Logger::println(LogLevel::Severe, "Editor", "Build action failed with an unknown error.");
            }

            m_succeeded = ok;
            m_finished = true;
        });
    }

    void BuildActions::start_build(const fs::path& projectRoot){
        start(BuildActionKind::Build, [projectRoot]{ run_cmake_build(projectRoot); });
    }

    void BuildActions::start_validate(const fs::path& projectRoot, Engine& sceneEngine){
        start(BuildActionKind::ValidateAssets, [projectRoot, &sceneEngine]{
            // Texture/Font/Model construction all issue real GL calls, same as validate/main.cpp.
            glfwInit();
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            RenderWindow window(64, 64, "draft_editor_validate");

            CwdGuard cwdGuard;
            fs::current_path(projectRoot);

            std::vector<AssetTask> tasks = collect_project_assets(projectRoot);
            AssetManager assets;
            auto errors = validate_assets(assets, sceneEngine, tasks);

            for(const AssetTask& task : tasks){
                auto it = errors.find(task.key);
                if(it == errors.end())
                    Logger::println(LogLevel::Info, asset_kind_name(task.kind), task.key + " - OK");
                else
                    Logger::println(LogLevel::Severe, asset_kind_name(task.kind), task.key + " - FAILED: " + it->second);
            }

            Logger::println(errors.empty() ? LogLevel::Info : LogLevel::Critical, "Validate",
                std::to_string(tasks.size() - errors.size()) + "/" + std::to_string(tasks.size()) + " assets valid");

            if(!errors.empty())
                throw std::runtime_error("Validate: " + std::to_string(errors.size()) + " asset(s) failed validation");
        });
    }

    void BuildActions::start_pack(const fs::path& projectRoot, const fs::path& outputPath){
        start(BuildActionKind::Pack, [projectRoot, outputPath]{
            if(!pack_project(projectRoot, outputPath))
                throw std::runtime_error("Pack: failed, see log above");
        });
    }

    void BuildActions::start_export(const fs::path& projectRoot, const fs::path& outputDir){
        start(BuildActionKind::Export, [projectRoot, outputDir]{
            export_project(projectRoot, outputDir);
        });
    }

    BuildActionResult BuildActions::poll(){
        if(!m_thread || !m_finished.load())
            return BuildActionResult::None;

        m_thread->join();
        m_thread.reset();

        if(m_kind != BuildActionKind::Build)
            return BuildActionResult::OtherFinished;

        return m_succeeded.load() ? BuildActionResult::BuildSucceeded : BuildActionResult::BuildFailed;
    }
}
