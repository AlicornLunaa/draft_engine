#include "draft/editor/editor_application.hpp"
#include "draft/core/command_catalog.hpp"
#include "draft/ecs/scene_serializer.hpp"
#include "draft/editor/freecam_controller.hpp"
#include "draft/editor/panels/animation_editor_panel.hpp"
#include "draft/editor/panels/asset_browser_panel.hpp"
#include "draft/editor/panels/collider_gizmo.hpp"
#include "draft/editor/panels/dockspace_panel.hpp"
#include "draft/editor/panels/gizmo_overlay.hpp"
#include "draft/editor/panels/hierarchy_panel.hpp"
#include "draft/editor/panels/inspector_panel.hpp"
#include "draft/editor/panels/joint_gizmo.hpp"
#include "draft/editor/panels/particle_editor_panel.hpp"
#include "draft/editor/panels/rml_editor_panel.hpp"
#include "draft/editor/panels/settings_panel.hpp"
#include "draft/editor/panels/shader_editor_panel.hpp"
#include "draft/editor/panels/systems_panel.hpp"
#include "draft/editor/panels/viewport_panel.hpp"
#include "draft/interface/imgui/console_system.hpp"
#include "draft/interface/imgui/imgui_system.hpp"
#include "draft/interface/imgui/profiler_system.hpp"
#include "draft/interface/rmlui/rml_system.hpp"
#include "draft/util/files/file_handle.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/logger.hpp"
#include "draft/util/serialization/serializer.hpp"
#include "draft/input/action.hpp"

namespace Draft {
    EditorApplication::EditorApplication(const std::string& title, unsigned int width, unsigned int height)
        : application(title, width, height), gameApp({width, height})
    {
        gameApp.simulationPaused = true;
        RmlUiSystem::set_clipboard_window(application.window);
        application.set_scene(&editScene);
        gameApp.set_scene(&gameScene);
        attach_chrome();
        register_editor_commands();
    }

    bool EditorApplication::step(){
        if(m_watcher && m_watcher->poll())
            m_pending = PendingAction::ReloadModule;

        switch(m_buildActions.poll()){
            case BuildActionResult::BuildSucceeded:
                m_pending = PendingAction::ReloadModule;
                break;

            case BuildActionResult::BuildFailed:
                m_playAfterBuild = false;
                break;

            case BuildActionResult::None:
            case BuildActionResult::OtherFinished:
                break;
        }

        bool open = application.step();

        // Send any built up events to the sub app
        while(!pendingViewportEvents.empty()){
            Event& event = pendingViewportEvents.front();
            FakeMouse& mouse = gameApp.fakeMouse;
            FakeKeyboard& keyboard = gameApp.fakeKeyboard;

            switch(event.type){
                case Event::Resized:
                    gameApp.resize({event.size.width, event.size.height});
                    break;

                case Event::MouseMoved:
                    mouse.position_changed(event.mouseMove.x, event.mouseMove.y);
                    break;

                case Event::MouseButtonPressed:
                case Event::MouseButtonReleased:
                    mouse.button_pressed(event.mouseButton.button, event.type == Event::MouseButtonPressed ? Action::PRESS : Action::RELEASE, event.mouseButton.mods);
                    break;

                case Event::MouseWheelScrolled:
                    mouse.mouse_scrolled(event.mouseWheelScroll.x, event.mouseWheelScroll.y);
                    break;

                case Event::MouseEntered:
                    mouse.mouse_entered(1);
                    break;

                case Event::MouseLeft:
                    mouse.mouse_entered(0);
                    break;

                case Event::KeyPressed:
                case Event::KeyReleased:
                case Event::KeyHold:
                    keyboard.key_press(event.key.code, event.type == Event::KeyPressed ? Action::PRESS : event.type == Event::KeyReleased ? Action::RELEASE : Action::HOLD, event.key.mods);
                    break;

                case Event::TextEntered:
                    keyboard.text_entered(event.text.unicode);
                    break;

                default:
                    break;
            }

            gameApp.inject_event(event); // Sub application doesnt have its own callback hooking
            pendingViewportEvents.pop();
        }
        gameApp.step(application.deltaTime);

        process_pending();
        return open;
    }

    void EditorApplication::run(){
        while(step());
    }

    void EditorApplication::request_open_project(std::filesystem::path root){
        m_pendingProjectPath = std::move(root);
        m_pending = PendingAction::OpenProject;
    }

    void EditorApplication::request_reload_module(){
        m_pending = PendingAction::ReloadModule;
    }

    void EditorApplication::request_build(){
        if(!m_project)
            return;

        m_buildActions.start_build(m_project->root());
    }

    void EditorApplication::request_build_and_play(){
        if(!m_project)
            return;

        m_playAfterBuild = true;
        m_buildActions.start_build(m_project->root());
    }

    void EditorApplication::request_validate_assets(){
        if(!m_project)
            return;

        m_buildActions.start_validate(m_project->root(), gameEngine);
    }

    void EditorApplication::request_pack(){
        if(!m_project)
            return;

        std::filesystem::path outputPath = m_project->root() / "build" / "assets.apak";
        m_buildActions.start_pack(m_project->root(), outputPath);
    }

    void EditorApplication::request_export(std::filesystem::path outputDir){
        if(!m_project)
            return;

        m_buildActions.start_export(m_project->root(), std::move(outputDir));
    }

    void EditorApplication::request_play(){
        m_pending = PendingAction::Play;
    }

    void EditorApplication::request_stop(){
        m_pending = PendingAction::Stop;
    }

    void EditorApplication::request_open_scene(std::filesystem::path path){
        m_pendingScenePath = std::move(path);
        m_pending = PendingAction::OpenScene;
    }

    void EditorApplication::request_new_scene(std::filesystem::path path){
        m_pendingScenePath = std::move(path);
        m_pending = PendingAction::NewScene;
    }

    void EditorApplication::save_scene_to(const std::filesystem::path& path){
        HostFileSystem().create_directories(path);
        save_scene(gameScene, gameEngine, assets, HostFileSystem().open(path));
        currentScenePath = path;
    }

    void EditorApplication::toggle_pause(){
        if(!m_isPlaying)
            return;

        gameApp.simulationPaused = !gameApp.simulationPaused;
    }

    void EditorApplication::process_pending(){
        PendingAction action = m_pending;
        m_pending = PendingAction::None;

        try {
            switch(action){
                case PendingAction::None: break;
                case PendingAction::OpenProject: open_project(m_pendingProjectPath); break;
                case PendingAction::ReloadModule:
                    load_game_module(true);
                    if(m_playAfterBuild){
                        m_playAfterBuild = false;
                        play();
                    }
                    break;
                case PendingAction::Play: play(); break;
                case PendingAction::Stop: stop(); break;
                case PendingAction::OpenScene: open_scene_now(m_pendingScenePath); break;
                case PendingAction::NewScene: new_scene_now(m_pendingScenePath); break;
            }
        } catch(const std::exception& e){
            Logger::println(LogLevel::Severe, "Editor", e.what());
        }
    }

    void EditorApplication::open_project(const std::filesystem::path& root){
        m_project.emplace(root);
        load_settings();
        load_game_module(false);
    }

    void EditorApplication::load_settings(){
        settings = EditorSettings();

        if(!m_project)
            return;

        FileHandle manifest = HostFileSystem().open(m_project->manifest_path());
        if(!manifest.exists())
            return;

        JSON json(manifest);
        if(json.contains("editor"))
            Serializer::deserialize(settings, json["editor"]);
    }

    void EditorApplication::save_settings(){
        if(!m_project)
            return;

        FileHandle manifest = HostFileSystem().open(m_project->manifest_path());

        // Preserve whatever else might already live in the manifest (module/asset location, ...)
        // rather than clobbering it with a file containing only "editor".
        JSON json = manifest.exists() ? JSON(manifest) : JSON::object();
        Serializer::serialize(settings, json["editor"]);

        manifest.write_string(json.dump(4));
    }

    void EditorApplication::load_game_module(bool preserveScene){
        if(!m_project)
            return;

        std::filesystem::path modulePath = m_project->resolved_module_path();

        // Games resolve their own asset paths (e.g. "assets/dev_texture.png") relative to the
        // process's working directory, the same directory the module itself lives in (an
        // "assets" symlink/copy sits next to it, see test_bench/CMakeLists.txt's POST_BUILD step).
        std::filesystem::current_path(modulePath.parent_path());

        bool wasPlaying = preserveScene && m_isPlaying;
        std::filesystem::path reloadSnapshotPath = wasPlaying ? snapshot_path() : reload_snapshot_path();

        if(preserveScene && !wasPlaying){
            HostFileSystem().create_directories(reloadSnapshotPath);
            save_scene(gameScene, gameEngine, assets, HostFileSystem().open(reloadSnapshotPath));
        }

        GameModuleLoader newModule(modulePath);

        gameApp.simulationPaused = true;
        m_isPlaying = false;
        selection.clear();
        gameScene.get_registry().clear();
        gameScene.get_systems().clear();
        gameEngine.clear();

        m_gameModule = std::move(newModule);
        m_gameModule->register_game(gameContext, gameScene);
        m_watcher.emplace(modulePath);

        if(preserveScene){
            // Discard whatever register_game() just seeded on its own and restore the snapshot
            // taken above instead, against the freshly (re)registered catalogs
            gameScene.get_registry().clear();
            gameScene.get_systems().clear();
            load_scene(gameScene, gameEngine, assets, HostFileSystem().open(reloadSnapshotPath));
        }

        // Reload always leaves simulationPaused/m_isPlaying reset above (the module swap itself
        // requires it), so resume here
        if(wasPlaying)
            play();

        Logger::println(LogLevel::Info, "Editor", "Loaded game module " + modulePath.string());
    }

    void EditorApplication::attach_chrome(){
        editScene.get_systems().add<ImGuiSystem>(application.target.get_size(), "imgui_editor.ini", false);
        editScene.get_systems().add<DockspacePanelSystem>(*this);
        editScene.get_systems().add<ViewportPanelSystem>(*this);
        editScene.get_systems().add<ParticleEditorPanelSystem>(*this);
        editScene.get_systems().add<AnimationEditorPanelSystem>(*this);
        editScene.get_systems().add<ShaderEditorPanelSystem>(*this);
        editScene.get_systems().add<RmlEditorPanelSystem>(*this);
        editScene.get_systems().add<FreecamControllerSystem>(*this);
        editScene.get_systems().add<HierarchyPanelSystem>(*this);
        editScene.get_systems().add<InspectorPanelSystem>(*this);
        editScene.get_systems().add<SystemsPanelSystem>(*this);
        editScene.get_systems().add<SettingsPanelSystem>(*this);
        editScene.get_systems().add<AssetBrowserPanelSystem>(*this);
        editScene.get_systems().add<ConsoleSystem>(gameEngine, gameApp, assets);
        editScene.get_systems().add<ProfilerSystem>();

        // ColliderGizmoSystem must render before GizmoOverlaySystem because it sets
        // colliderGizmoActiveThisFrame fresh each frame.
        editScene.get_systems().add<ColliderGizmoSystem>(*this);
        editScene.get_systems().add<GizmoOverlaySystem>(*this);
        editScene.get_systems().add<JointGizmoSystem>(*this);
    }

    void EditorApplication::register_editor_commands(){
        gameEngine.register_command({
            "play", "Starts play mode.",
            [this](CommandContext&){ request_play(); }
        });

        gameEngine.register_command({
            "stop", "Stops play mode.",
            [this](CommandContext&){ request_stop(); }
        });

        gameEngine.register_command({
            "pause", "Toggles simulation pause.",
            [this](CommandContext&){ toggle_pause(); }
        });
    }

    void EditorApplication::play(){
        if(!m_project)
            return;

        std::filesystem::path path = snapshot_path();
        HostFileSystem().create_directories(path);
        save_scene(gameScene, gameEngine, assets, HostFileSystem().open(path));
        gameApp.simulationPaused = false;
        m_isPlaying = true;
    }

    void EditorApplication::stop(){
        if(!m_project)
            return;

        gameApp.simulationPaused = true;
        m_isPlaying = false;

        FileHandle snapshot = HostFileSystem().open(snapshot_path());
        if(!snapshot.exists())
            return;

        selection.clear();
        gameScene.get_registry().clear();
        gameScene.get_systems().clear();

        load_scene(gameScene, gameEngine, assets, snapshot);
    }

    void EditorApplication::open_scene_now(const std::filesystem::path& path){
        FileHandle file = HostFileSystem().open(path);
        if(!file.exists()){
            Logger::println(LogLevel::Severe, "Editor", "Scene not found: " + path.string());
            return;
        }

        selection.clear();
        gameScene.get_registry().clear();
        gameScene.get_systems().clear();

        try {
            load_scene(gameScene, gameEngine, assets, file);
            currentScenePath = path;
        } catch(const std::exception& e){
            Logger::println(LogLevel::Severe, "Editor", "Failed to open scene " + path.string() + ": " + e.what());
        }
    }

    void EditorApplication::new_scene_now(const std::filesystem::path& path){
        HostFileSystem().create_directories(path);

        Scene empty;
        save_scene(empty, gameEngine, assets, HostFileSystem().open(path));

        open_scene_now(path);
    }

    std::filesystem::path EditorApplication::snapshot_path() const {
        return m_project->root() / ".draft-editor" / "play_snapshot.json";
    }

    std::filesystem::path EditorApplication::reload_snapshot_path() const {
        return m_project->root() / ".draft-editor" / "reload_snapshot.json";
    }
}
