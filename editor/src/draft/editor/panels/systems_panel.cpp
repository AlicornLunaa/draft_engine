#include "draft/editor/panels/systems_panel.hpp"
#include "draft/ecs/system_catalog.hpp"
#include "draft/editor/editor_application.hpp"

#include "imgui.h"

namespace Draft {
    SystemsPanelSystem::SystemsPanelSystem(EditorApplication& app) : m_app(app) {}

    void SystemsPanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default || !m_app.systemsPanelVisible)
            return;

        ImGui::SetNextWindowSize({320, 240}, ImGuiCond_FirstUseEver);

        if(ImGui::Begin("Systems", &m_app.systemsPanelVisible)){
            SystemRegistry& registry = m_app.gameScene.get_systems();

            for(SystemTypeInterface* entry : m_app.gameEngine.systems().all()){
                bool attached = entry->has(registry);

                if(ImGui::Checkbox(entry->name().c_str(), &attached)){
                    if(attached)
                        entry->add(m_app.gameScene);
                    else
                        entry->remove(registry);
                }
            }
        }

        ImGui::End();
    }
}
