#include "draft/editor/panels/viewport_panel.hpp"
#include "draft/editor/editor_application.hpp"

#include "imgui.h"

#include <cstdint>

namespace Draft {
    ViewportPanelSystem::ViewportPanelSystem(EditorApplication& app) : m_app(app) {}

    void ViewportPanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default)
            return;

        ImGui::Begin("Viewport");

        ImVec2 avail = ImGui::GetContentRegionAvail();
        if(avail.x >= 1.f && avail.y >= 1.f)
            m_app.gameApp.resize({(unsigned int)avail.x, (unsigned int)avail.y});

        // The Framebuffer's color attachment is upright in GL texture space, ImGui expects
        // top-left origin, hence the flipped v coordinates.
        auto textureId = (ImTextureID)(intptr_t)m_app.gameApp.get_output().get_texture_handle();
        ImGui::Image(textureId, avail, ImVec2(0, 1), ImVec2(1, 0));

        ImGui::End();
    }
}
