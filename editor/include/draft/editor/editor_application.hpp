#pragma once

#include "draft/asset/asset_manager.hpp"
#include "draft/build_tools/game_context.hpp"
#include "draft/build_tools/game_module_loader.hpp"
#include "draft/core/application.hpp"
#include "draft/core/engine.hpp"
#include "draft/core/sub_application.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/editor/editor_settings.hpp"
#include "draft/editor/game_module_watcher.hpp"
#include "draft/editor/project.hpp"
#include "draft/editor/selection.hpp"
#include "draft/math/glm.hpp"

#include <filesystem>
#include <optional>
#include <queue>
#include <string>

namespace Draft {
    /**
     * @brief Which shape (if any) of the selected entity's Collider is being edited
     */
    struct ColliderShapeSelection {
        bool editing = false;
        int shapeIndex = -1;
    };

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

        /**
         * @brief Serializes `settings` to EditorProject::manifest_path(). Synchronous, no
         * scene/registry state involved, safe to call directly from a panel's render() (e.g. on
         * every settings field edit).
         */
        void save_settings();

        /**
         * @brief Clears gameScene and loads @p path into it, tracking it as currentScenePath.
         * Deferred to the next step() (see class doc comment), same as Play/Stop, since it
         * mutates gameScene's registry/systems.
         */
        void request_open_scene(std::filesystem::path path);

        /**
         * @brief Writes a brand new, empty scene to @p path, then opens it the same way
         * request_open_scene() would. Deferred for the same reason.
         */
        void request_new_scene(std::filesystem::path path);

        /**
         * @brief Serializes gameScene to @p path and tracks it as currentScenePath. Synchronous -
         * unlike open/new, this only reads gameScene, it never touches its registry/systems, so
         * there's no reentrancy hazard calling it directly from a panel's render().
         */
        void save_scene_to(const std::filesystem::path& path);

        /**
         * @brief True from request_play() until request_stop(), regardless of gameApp.simulationPaused
         */
        bool is_playing() const { return m_isPlaying; }

        /**
         * @brief Toggles gameApp.simulationPaused directly unlike Play/Stop, this touches no
         * scene state (no snapshot save, no registry/systems clear)
         */
        void toggle_pause();

        Application application;
        AssetManager assets;
        Scene editScene;

        Engine gameEngine;
        SubApplication gameApp;
        GameContext gameContext{gameEngine, gameApp, assets};
        Scene gameScene;

        EditorSelection selection;

        bool viewportFocused = false;
        bool viewportHovered = false;
        std::queue<Event> pendingViewportEvents;

        /**
         * @brief The Viewport window's content region, in absolute screen pixels.
         */
        Vector2d viewportScreenPosition{};
        Vector2d viewportSize{1, 1};

        ColliderShapeSelection colliderShapeSelection;

        /**
         * @brief Whether the Systems panel window is open, toggled from the dockspace's View menu.
         */
        bool systemsPanelVisible = true;

        /**
         * @brief Whether the Settings panel window is open, toggled from the dockspace's View menu.
         */
        bool settingsPanelVisible = false;

        /**
         * @brief Editor preferences (snap distances, ...), loaded from and saved to
         * EditorProject::manifest_path().
         */
        EditorSettings settings;

        /**
         * @brief Reset to false at the top of ColliderGizmoSystem::render() every frame, set true
         * if any of its handles were active or it just consumed a ghost-point insert click.
         */
        bool colliderGizmoActiveThisFrame = false;

        /**
         * @brief Absolute path gameScene was last opened/saved from, or nullopt if it's never
         * been saved anywhere yet ("(Unsaved scene)" in the dockspace's title text).
         */
        std::optional<std::filesystem::path> currentScenePath;

    private:
        enum class PendingAction {
            None,
            OpenProject,
            ReloadModule,
            Play,
            Stop,
            OpenScene,
            NewScene
        };

        void process_pending();
        void open_project(const std::filesystem::path& root);
        void load_settings();
        void load_game_module();
        void attach_chrome();
        void register_editor_commands();
        void play();
        void stop();
        void open_scene_now(const std::filesystem::path& path);
        void new_scene_now(const std::filesystem::path& path);
        std::filesystem::path snapshot_path() const;

        std::optional<EditorProject> m_project;
        std::optional<GameModuleLoader> m_gameModule;
        std::optional<GameModuleWatcher> m_watcher;

        PendingAction m_pending = PendingAction::None;
        std::filesystem::path m_pendingProjectPath;
        std::filesystem::path m_pendingScenePath;
        bool m_isPlaying = false;
    };
}
