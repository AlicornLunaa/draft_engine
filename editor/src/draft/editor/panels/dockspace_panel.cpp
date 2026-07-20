#include "draft/editor/panels/dockspace_panel.hpp"
#include "draft/editor/editor_application.hpp"

#include "imgui.h"
#include "imgui_internal.h"

#include <cstring>
#include <filesystem>

namespace Draft {
    DockspacePanelSystem::DockspacePanelSystem(EditorApplication& app) : m_app(app) {
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        if(app.has_project()){
            std::string root = app.project()->root().string();
            std::strncpy(m_projectPathBuffer.data(), root.c_str(), m_projectPathBuffer.size() - 1);
        }
    }

    void DockspacePanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default)
            return;

        ImGuiID dockspaceId = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        if(!m_dockspaceBuilt){
            m_dockspaceBuilt = true;
            build_initial_layout(dockspaceId);
        }

        if(ImGui::BeginMainMenuBar()){
            draw_menu_bar();
            draw_play_controls();
            ImGui::EndMainMenuBar();
        }

        if(m_openScenePopupRequested){
            m_openScenePopupRequested = false;
            ImGui::OpenPopup("Scene Path");
        }

        draw_scene_path_modal();
    }

    void DockspacePanelSystem::draw_menu_bar(){
        if(ImGui::BeginMenu("File")){
            ImGui::SetNextItemWidth(300.f);
            ImGui::InputText("##ProjectPath", m_projectPathBuffer.data(), m_projectPathBuffer.size());
            ImGui::SameLine();

            if(ImGui::MenuItem("Open Project"))
                m_app.request_open_project(std::filesystem::path(m_projectPathBuffer.data()));

            ImGui::BeginDisabled(!m_app.has_project());
            if(ImGui::MenuItem("Reload Module"))
                m_app.request_reload_module();
            ImGui::EndDisabled();

            ImGui::Separator();

            ImGui::BeginDisabled(!m_app.has_project());
            if(ImGui::MenuItem("New Scene"))
                open_scene_prompt(ScenePromptMode::New);

            if(ImGui::MenuItem("Save Scene")){
                if(m_app.currentScenePath)
                    m_app.save_scene_to(*m_app.currentScenePath);
                else
                    open_scene_prompt(ScenePromptMode::SaveAs);
            }

            if(ImGui::MenuItem("Save Scene As..."))
                open_scene_prompt(ScenePromptMode::SaveAs);
            ImGui::EndDisabled();

            ImGui::Separator();
            if(ImGui::MenuItem("Exit"))
                m_app.application.window.close();

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Build")){
            // Wired up in Phase 7
            ImGui::MenuItem("Build Project", nullptr, false, false);
            ImGui::MenuItem("Export Project", nullptr, false, false);
            ImGui::EndMenu();
        }
    }

    void DockspacePanelSystem::draw_play_controls(){
        ImGui::BeginDisabled(!m_app.has_project());

        if(!m_app.is_playing()){
            if(ImGui::MenuItem("Play"))
                m_app.request_play();
        } else {
            if(m_app.gameApp.simulationPaused){
                if(ImGui::MenuItem("Resume"))
                    m_app.toggle_pause();
            } else {
                if(ImGui::MenuItem("Pause"))
                    m_app.toggle_pause();
            }

            if(ImGui::MenuItem("Stop"))
                m_app.request_stop();
        }

        ImGui::EndDisabled();

        if(m_app.has_project()){
            ImGui::Text("  %s", m_app.project()->root().string().c_str());
            ImGui::SameLine();

            if(m_app.currentScenePath)
                ImGui::Text(" | %s", m_app.currentScenePath->filename().string().c_str());
            else
                ImGui::TextDisabled(" | (Unsaved scene)");
        }
    }

    void DockspacePanelSystem::open_scene_prompt(ScenePromptMode mode){
        m_scenePrompt = mode;

        std::filesystem::path suggested = m_app.currentScenePath
            ? *m_app.currentScenePath
            : (m_app.project()->assets_dir() / "scenes" / "NewScene.json");

        std::string suggestedStr = suggested.string();
        std::strncpy(m_scenePathBuffer.data(), suggestedStr.c_str(), m_scenePathBuffer.size() - 1);

        m_openScenePopupRequested = true;
    }

    void DockspacePanelSystem::draw_scene_path_modal(){
        if(ImGui::BeginPopupModal("Scene Path", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
            ImGui::Text("%s", m_scenePrompt == ScenePromptMode::New ? "New Scene" : "Save Scene As");
            ImGui::SetNextItemWidth(400.f);
            ImGui::InputText("##ScenePath", m_scenePathBuffer.data(), m_scenePathBuffer.size());

            if(ImGui::Button("Save")){
                std::filesystem::path path(m_scenePathBuffer.data());

                if(m_scenePrompt == ScenePromptMode::New)
                    m_app.request_new_scene(path);
                else
                    m_app.save_scene_to(path);

                m_scenePrompt = ScenePromptMode::None;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if(ImGui::Button("Cancel")){
                m_scenePrompt = ScenePromptMode::None;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void DockspacePanelSystem::build_initial_layout(unsigned int dockspaceId){
        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(dockspaceId, (ImGuiDockNodeFlags)ImGuiDockNodeFlags_PassthruCentralNode | (ImGuiDockNodeFlags)ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);

        ImGuiID hierarchyId;
        ImGuiID rightOfHierarchyId;
        ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.2f, &hierarchyId, &rightOfHierarchyId);

        ImGuiID assetBrowserId;
        ImGui::DockBuilderSplitNode(hierarchyId, ImGuiDir_Down, 0.4f, &assetBrowserId, &hierarchyId);

        ImGuiID inspectorId;
        ImGuiID viewportId;
        ImGui::DockBuilderSplitNode(rightOfHierarchyId, ImGuiDir_Right, 0.3f, &inspectorId, &viewportId);

        ImGui::DockBuilderDockWindow("Hierarchy", hierarchyId);
        ImGui::DockBuilderDockWindow("Asset Browser", assetBrowserId);
        ImGui::DockBuilderDockWindow("Viewport###Viewport", viewportId);
        ImGui::DockBuilderDockWindow("Inspector", inspectorId);

        ImGui::DockBuilderFinish(dockspaceId);
    }
}
