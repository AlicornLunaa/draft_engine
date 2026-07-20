#include "draft/editor/panels/settings_panel.hpp"
#include "draft/editor/editor_application.hpp"

#include "imgui.h"

namespace Draft {
    SettingsPanelSystem::SettingsPanelSystem(EditorApplication& app) : m_app(app) {}

    void SettingsPanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default || !m_app.settingsPanelVisible)
            return;

        ImGui::SetNextWindowSize({320, 160}, ImGuiCond_FirstUseEver);

        if(ImGui::Begin("Settings", &m_app.settingsPanelVisible)){
            if(!m_app.has_project()){
                ImGui::TextDisabled("No project open");
            } else {
                ImGui::TextDisabled("Gizmo snapping");

                bool changed = false;
                changed |= ImGui::DragFloat("Position snap step", &m_app.settings.positionSnapStep, 0.5f, 0.01f, 1000.f);
                changed |= ImGui::DragFloat("Rotation snap step (degrees)", &m_app.settings.rotationSnapStepDegrees, 0.5f, 0.01f, 360.f);

                if(changed)
                    m_app.save_settings();
            }
        }

        ImGui::End();
    }
}
