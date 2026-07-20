#include "draft/editor/panels/dockspace_panel.hpp"
#include "draft/editor/editor_application.hpp"

#include "imgui.h"
#include "imgui_internal.h"

#include <cmath>
#include <cstring>
#include <filesystem>

namespace Draft {
    namespace {
        // Rotating-arc throbber for indeterminate waits.
        void draw_throbber(const char* label, float radius, float thickness){
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if(window->SkipItems)
                return;

            ImGuiContext& g = *GImGui;
            const ImGuiStyle& style = g.Style;
            const ImGuiID id = window->GetID(label);

            ImVec2 pos = window->DC.CursorPos;
            ImVec2 size(radius * 2.f, (radius + style.FramePadding.y) * 2.f);

            const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
            ImGui::ItemSize(bb, style.FramePadding.y);
            if(!ImGui::ItemAdd(bb, id))
                return;

            window->DrawList->PathClear();

            const int numSegments = 30;
            const int start = static_cast<int>(std::abs(std::sin(g.Time * 1.8f)) * (numSegments - 5));
            const float aMin = IM_PI * 2.0f * float(start) / float(numSegments);
            const float aMax = IM_PI * 2.0f * float(numSegments - 3) / float(numSegments);
            const ImVec2 centre(pos.x + radius, pos.y + radius + style.FramePadding.y);

            for(int i = 0; i < numSegments; i++){
                const float a = aMin + (float(i) / float(numSegments)) * (aMax - aMin);
                window->DrawList->PathLineTo(ImVec2(
                    centre.x + std::cos(a + g.Time * 8.f) * radius,
                    centre.y + std::sin(a + g.Time * 8.f) * radius
                ));
            }

            window->DrawList->PathStroke(ImGui::GetColorU32(ImGuiCol_ButtonHovered), false, thickness);
        }
    }

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

        handle_global_shortcuts();

        if(m_openScenePopupRequested){
            m_openScenePopupRequested = false;
            ImGui::OpenPopup("Scene Path");
        }

        if(m_openExportPopupRequested){
            m_openExportPopupRequested = false;
            ImGui::OpenPopup("Export Path");
        }

        draw_scene_path_modal();
        draw_export_path_modal();
        draw_build_progress_modal();
    }

    void DockspacePanelSystem::draw_menu_bar(){
        bool busy = m_app.build_action_running();

        if(ImGui::BeginMenu("File")){
            ImGui::SetNextItemWidth(300.f);
            ImGui::InputText("##ProjectPath", m_projectPathBuffer.data(), m_projectPathBuffer.size());
            ImGui::SameLine();

            ImGui::BeginDisabled(busy);
            if(ImGui::MenuItem("Open Project"))
                m_app.request_open_project(std::filesystem::path(m_projectPathBuffer.data()));
            ImGui::EndDisabled();

            ImGui::BeginDisabled(!m_app.has_project() || busy);
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
            ImGui::BeginDisabled(!m_app.has_project() || busy);

            if(ImGui::MenuItem("Build"))
                do_build();

            if(ImGui::MenuItem("Validate Assets"))
                m_app.request_validate_assets();

            if(ImGui::MenuItem("Pack Assets"))
                m_app.request_pack();

            if(ImGui::MenuItem("Export Project...")){
                std::string suggested = (m_app.project()->root() / "export").string();
                std::strncpy(m_exportPathBuffer.data(), suggested.c_str(), m_exportPathBuffer.size() - 1);
                m_openExportPopupRequested = true;
            }

            ImGui::EndDisabled();
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("View")){
            ImGui::MenuItem("Hierarchy", nullptr, &m_app.hierarchyPanelVisible);
            ImGui::MenuItem("Asset Browser", nullptr, &m_app.assetBrowserPanelVisible);
            ImGui::MenuItem("Viewport", nullptr, &m_app.viewportPanelVisible);
            ImGui::MenuItem("Inspector", nullptr, &m_app.inspectorPanelVisible);
            ImGui::MenuItem("Systems", nullptr, &m_app.systemsPanelVisible);
            ImGui::MenuItem("Settings", nullptr, &m_app.settingsPanelVisible);
            ImGui::EndMenu();
        }

        if(busy)
            ImGui::TextDisabled("  %s", m_app.build_action_label().c_str());
    }

    void DockspacePanelSystem::do_build(){
        m_app.request_build();
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

    void DockspacePanelSystem::handle_global_shortcuts(){
        if(!m_app.has_project())
            return;

        ImGuiIO& io = ImGui::GetIO();
        bool busy = m_app.build_action_running();

        if(!busy && ImGui::IsKeyPressed(ImGuiKey_F5, false) && !m_app.is_playing())
            m_app.request_build_and_play();

        if(!busy && io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_B, false))
            do_build();

        if(io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_S, false)){
            open_scene_prompt(ScenePromptMode::SaveAs);
        } else if(io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)){
            if(m_app.currentScenePath)
                m_app.save_scene_to(*m_app.currentScenePath);
            else
                open_scene_prompt(ScenePromptMode::SaveAs);
        }

        if(io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_Comma, false))
            m_app.settingsPanelVisible = !m_app.settingsPanelVisible;

        // Guarded by WantTextInput so Delete still deletes a character while editing an
        // InputText (e.g. a Tag) instead of also deleting the whole selection.
        if(!io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Delete, false) && m_app.selection.count() > 0){
            for(Entity entity : m_app.selection.all())
                entity.destroy();

            m_app.selection.clear();
        }
    }

    void DockspacePanelSystem::open_scene_prompt(ScenePromptMode mode){
        m_scenePrompt = mode;

        std::filesystem::path suggested = m_app.currentScenePath
            ? *m_app.currentScenePath
            : (m_app.project()->assets_dir() / "scenes" / "NewScene.scene");

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

    void DockspacePanelSystem::draw_export_path_modal(){
        if(ImGui::BeginPopupModal("Export Path", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
            ImGui::Text("Export Project");
            ImGui::SetNextItemWidth(400.f);
            ImGui::InputText("##ExportPath", m_exportPathBuffer.data(), m_exportPathBuffer.size());

            if(ImGui::Button("Export")){
                m_app.request_export(std::filesystem::path(m_exportPathBuffer.data()));
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if(ImGui::Button("Cancel"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
        }
    }

    void DockspacePanelSystem::draw_build_progress_modal(){
        // OpenPopup only while running (calling it every frame is harmless, but pointless once
        // finished); BeginPopupModal still runs every frame the popup is open so the one frame
        // running() just went false can reach CloseCurrentPopup() below.
        if(m_app.build_action_running())
            ImGui::OpenPopup("Build Progress");

        ImGui::SetNextWindowSize({320, 0}, ImGuiCond_Always);
        if(ImGui::BeginPopupModal("Build Progress", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)){
            if(!m_app.build_action_running()){
                ImGui::CloseCurrentPopup();
            } else {
                ImGui::TextUnformatted(m_app.build_action_label().c_str());

                const float radius = 16.f;
                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - radius * 2.f) * 0.5f);
                draw_throbber("##build_throbber", radius, 4.f);
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

        // Joins Inspector's dock node as a second, initially unfocused tab.
        ImGui::DockBuilderDockWindow("Systems", inspectorId);

        ImGui::DockBuilderFinish(dockspaceId);
    }
}
