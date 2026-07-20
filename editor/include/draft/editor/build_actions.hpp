#pragma once

#include <atomic>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <thread>

namespace Draft {
    class Engine;

    enum class BuildActionKind {
        Build,
        ValidateAssets,
        Pack,
        Export
    };

    enum class BuildActionResult {
        None,
        BuildSucceeded,
        BuildFailed,
        OtherFinished
    };

    /**
     * @brief Runs one Build/Validate/Pack/Export action at a time on a background thread, so a
     * multi-second `cmake --build` or asset pack doesn't freeze the editor. Wraps BuildTools'
     * own validate_assets()/pack_project()/export_project() verbatim, all progress and
     * failures are logged through Logger).
     *
     * Meant to be polled once per frame from EditorApplication::step(), the same pattern as
     * GameModuleWatcher. Only one action runs at a time; starting another while one is already
     * running is a no-op (logged as a warning).
     *
     * Caveat inherited from the wrapped functions, not introduced here: validate_assets()/
     * pack_project() each briefly chdir() the whole process for their duration (see
     * project_packer.cpp's CwdGuard), which is process-wide state, not per-thread. Harmless as
     * long as nothing else resolves a relative path at the same moment.
     */
    class BuildActions {
    public:
        ~BuildActions();

        bool running() const { return m_thread.has_value(); }
        BuildActionKind current_kind() const { return m_kind; }
        std::string label() const;

        void start_build(const std::filesystem::path& projectRoot);
        void start_validate(const std::filesystem::path& projectRoot, Engine& sceneEngine);
        void start_pack(const std::filesystem::path& projectRoot, const std::filesystem::path& outputPath);
        void start_export(const std::filesystem::path& projectRoot, const std::filesystem::path& outputDir);

        /**
         * @brief Joins the worker thread once it's finished; a no-op otherwise.
         * @return What finished, if anything, on this call. BuildSucceeded is what
         * EditorApplication uses to trigger Phase 1's module-reload path.
         */
        BuildActionResult poll();

    private:
        void start(BuildActionKind kind, std::function<void()> work);

        BuildActionKind m_kind = BuildActionKind::Build;
        std::optional<std::thread> m_thread;
        std::atomic<bool> m_finished{false};
        std::atomic<bool> m_succeeded{false};
    };
}
