#define GLFW_INCLUDE_NONE

#include "draft/editor/editor_application.hpp"
#include "draft/ecs/scene_serializer.hpp"
#include "draft/editor/panels/dockspace_panel.hpp"
#include "draft/editor/panels/hierarchy_panel.hpp"
#include "draft/editor/panels/viewport_panel.hpp"
#include "draft/interface/imgui/imgui_system.hpp"
#include "draft/interface/rmlui/rml_system.hpp"
#include "draft/util/files/file_handle.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/logger.hpp"

namespace Draft {
    EditorApplication::EditorApplication(const std::string& title, unsigned int width, unsigned int height)
        : application(title, width, height), gameView({.size={width, height}}), gameApp({width, height}, application.keyboard, application.mouse)
    {
        gameApp.simulationPaused = true;
        RmlUiSystem::set_clipboard_window(application.window);
        application.set_scene(&editScene);
        gameApp.set_scene(&gameScene);
        attach_chrome();

        // Mouse input is remapped/forwarded per-frame by ViewportPanelSystem itself (it needs
        // the panel's screen-space rect). Keyboard/text has no per-frame coordinates to remap,
        // so it's simplest forwarded straight from here, gated on viewportFocused.
        // application.eventCallback = [this](const Event& event){ return forward_keyboard_event(event); };
    }

    // bool EditorApplication::forward_keyboard_event(const Event& event){
    //     if(!viewportFocused){
    //         if(event.type == Event::TextEntered || event.type == Event::KeyPressed)
    //             Logger::println(LogLevel::Info, "Editor", "forward_keyboard_event: dropped, viewport not focused");
    //         return false;
    //     }

    //     switch(event.type){
    //         case Event::KeyPressed:
    //         case Event::KeyReleased:
    //         case Event::KeyHold: {
    //             gameApp.keyboard.inject_key(event.key.code, event.type == Event::KeyPressed ? GLFW_PRESS : event.type == Event::KeyReleased ? GLFW_RELEASE : GLFW_REPEAT, event.key.mods);
    //             bool consumed = gameApp.inject_event(event);
    //             Logger::println(LogLevel::Info, "Editor", "forward_keyboard_event: key code=" + std::to_string(event.key.code) + " consumed=" + std::to_string(consumed));
    //             return false;
    //         }

    //         case Event::TextEntered: {
    //             gameApp.keyboard.inject_char(event.text.unicode);
    //             bool consumed = gameApp.inject_event(event);
    //             Logger::println(LogLevel::Info, "Editor", "forward_keyboard_event: text unicode=" + std::to_string(event.text.unicode) + " consumed=" + std::to_string(consumed));
    //             return false;
    //         }

    //         default:
    //             return false;
    //     }
    // }

    bool EditorApplication::step(){
        if(m_watcher && m_watcher->poll())
            m_pending = PendingAction::ReloadModule;

        bool open = application.step();
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

    void EditorApplication::request_play(){
        m_pending = PendingAction::Play;
    }

    void EditorApplication::request_stop(){
        m_pending = PendingAction::Stop;
    }

    void EditorApplication::process_pending(){
        PendingAction action = m_pending;
        m_pending = PendingAction::None;

        try {
            switch(action){
                case PendingAction::None: break;
                case PendingAction::OpenProject: open_project(m_pendingProjectPath); break;
                case PendingAction::ReloadModule: load_game_module(); break;
                case PendingAction::Play: play(); break;
                case PendingAction::Stop: stop(); break;
            }
        } catch(const std::exception& e){
            Logger::println(LogLevel::Severe, "Editor", e.what());
        }
    }

    void EditorApplication::open_project(const std::filesystem::path& root){
        m_project.emplace(root);
        load_game_module();
    }

    void EditorApplication::load_game_module(){
        if(!m_project)
            return;

        std::filesystem::path modulePath = m_project->resolved_module_path();

        // Games resolve their own asset paths (e.g. "assets/dev_texture.png") relative to the
        // process's working directory, the same directory game.json and the module itself live
        // in.
        std::filesystem::current_path(m_project->module_manifest_path().parent_path());

        gameApp.simulationPaused = true;
        selection.clear();
        gameScene.get_registry().clear();
        gameScene.get_systems().clear();

        m_gameModule.emplace(modulePath);

        // register_game() constructs systems that create GL objects tied to gameApp's own
        // window/context (RmlUiSystem's VAOs, ImGuiSystem's font texture, ...). Those need to be
        // created with gameApp's own context current, not whatever's current at this point in
        // EditorApplication::step() (the shared/main one, since this runs from process_pending()
        // after gameApp.step() already restored it).
        // gameApp.make_current();
        m_gameModule->register_game(gameContext, gameScene);
        // gameApp.restore_shared_context();

        m_watcher.emplace(modulePath);

        Logger::println(LogLevel::Info, "Editor", "Loaded game module " + modulePath.string());
    }

    void EditorApplication::attach_chrome(){
        editScene.get_systems().add<ImGuiSystem>(application.target.get_size());
        editScene.get_systems().add<DockspacePanelSystem>(*this);
        editScene.get_systems().add<HierarchyPanelSystem>(*this);
        editScene.get_systems().add<ViewportPanelSystem>(*this);
    }

    void EditorApplication::play(){
        if(!m_project)
            return;

        std::filesystem::path path = snapshot_path();
        HostFileSystem().create_directories(path);
        save_scene(gameScene, gameEngine, assets, HostFileSystem().open(path));
        gameApp.simulationPaused = false;
    }

    void EditorApplication::stop(){
        if(!m_project)
            return;

        gameApp.simulationPaused = true;

        FileHandle snapshot = HostFileSystem().open(snapshot_path());
        if(!snapshot.exists())
            return;

        selection.clear();
        gameScene.get_registry().clear();
        gameScene.get_systems().clear();

        // Same reasoning as load_game_module(): load_scene() attaches systems through the same
        // factories register_game() uses, which create GL objects tied to gameApp's own context.
        // gameApp.make_current();
        load_scene(gameScene, gameEngine, assets, snapshot);
        // gameApp.restore_shared_context();
    }

    std::filesystem::path EditorApplication::snapshot_path() const {
        return m_project->root() / ".draft-editor" / "play_snapshot.json";
    }
}
