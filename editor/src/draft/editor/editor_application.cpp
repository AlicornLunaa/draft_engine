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
    }

    bool EditorApplication::step(){
        if(m_watcher && m_watcher->poll())
            m_pending = PendingAction::ReloadModule;

        bool open = application.step();

        // Send any built up events to the sub app
        while(!pendingViewportEvents.empty()){
            Event& event = pendingViewportEvents.front();
            FakeMouse& mouse = gameApp.fakeMouse;
            FakeKeyboard& keyboard = gameApp.fakeKeyboard;

            switch(event.type){
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
        m_gameModule->register_game(gameContext, gameScene);
        m_watcher.emplace(modulePath);

        Logger::println(LogLevel::Info, "Editor", "Loaded game module " + modulePath.string());
    }

    void EditorApplication::attach_chrome(){
        editScene.get_systems().add<ImGuiSystem>(application.target.get_size(), "imgui_editor.ini", false);
        editScene.get_systems().add<DockspacePanelSystem>(*this);
        editScene.get_systems().add<ViewportPanelSystem>(*this);
        // editScene.get_systems().add<HierarchyPanelSystem>(*this);
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

        load_scene(gameScene, gameEngine, assets, snapshot);
    }

    std::filesystem::path EditorApplication::snapshot_path() const {
        return m_project->root() / ".draft-editor" / "play_snapshot.json";
    }
}
