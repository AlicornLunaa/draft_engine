#pragma once

#include "draft/asset/asset_manager.hpp"
#include "draft/build_tools/game_context.hpp"
#include "draft/build_tools/game_module_loader.hpp"
#include "draft/core/application.hpp"
#include "draft/core/engine.hpp"
#include "draft/core/sub_application.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/editor/game_module_watcher.hpp"
#include "draft/editor/project.hpp"
#include "draft/editor/selection.hpp"
#include "draft/rendering/frame_buffer.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace Draft {
    /**
     * @brief Owns the editor's own window/chrome (dockspace, hierarchy, ...) on `editScene`, and
     * a `SubApplication` running the loaded project's own scene (`gameScene`) into an offscreen
     * Framebuffer.
     *
     * Actions requested from a panel (open a project, reload the module, play, stop) don't run
     * immediately, they're deferred to the next step() so a system's render() can safely request
     * one without destroying itself mid call (see request_open_project() and friends).
     */
    class EditorApplication {
    public:
        EditorApplication(const std::string& title, unsigned int width, unsigned int height);

        bool step();
        void run();

        bool has_project() const { return m_project.has_value(); }
        const EditorProject* project() const { return m_project ? &m_project.value() : nullptr; }

        void request_open_project(std::filesystem::path root);
        void request_reload_module();
        void request_play();
        void request_stop();

        Application application;
        AssetManager assets;
        Scene editScene;

        Framebuffer gameView;
        Engine gameEngine;
        SubApplication gameApp;
        GameContext gameContext{gameEngine, gameApp, assets};
        Scene gameScene;

        EditorSelection selection;

        /**
         * @brief Whether the "Viewport" panel currently has ImGui focus, updated once per frame
         * by ViewportPanelSystem.
         */
        bool viewportFocused = false;

        /**
         * @brief The "Viewport" panel's content region size this frame, updated by
         * ViewportPanelSystem::render() but deliberately not applied to gameApp until step()
         * calls gameApp.resize() itself, after application.step() (and the real GPU draw of this
         * frame's ImGui::Image() referencing gameApp's *current* output texture) has already
         * finished - see step()'s ordering comment for why applying it any earlier flickers.
         */
        Vector2u pendingViewportSize;

    private:
        enum class PendingAction {
            None,
            OpenProject,
            ReloadModule,
            Play,
            Stop
        };

        void process_pending();
        void open_project(const std::filesystem::path& root);
        void load_game_module();
        void attach_chrome();
        void play();
        void stop();
        std::filesystem::path snapshot_path() const;

        std::optional<EditorProject> m_project;
        std::optional<GameModuleLoader> m_gameModule;
        std::optional<GameModuleWatcher> m_watcher;

        PendingAction m_pending = PendingAction::None;
        std::filesystem::path m_pendingProjectPath;
    };
}
