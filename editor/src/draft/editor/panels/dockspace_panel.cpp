#include "draft/editor/panels/dockspace_panel.hpp"
#include "draft/editor/editor_application.hpp"

#include "imgui.h"

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

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        if(ImGui::BeginMainMenuBar()){
            draw_menu_bar();
            draw_play_controls();
            ImGui::EndMainMenuBar();
        }
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

        if(m_app.gameApp.simulationPaused){
            if(ImGui::MenuItem("Play"))
                m_app.request_play();
        } else {
            if(ImGui::MenuItem("Stop"))
                m_app.request_stop();
        }

        ImGui::EndDisabled();

        if(m_app.has_project())
            ImGui::Text("  %s", m_app.project()->root().string().c_str());
    }
}
