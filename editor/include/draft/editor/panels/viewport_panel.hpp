#pragma once

#include "draft/ecs/system.hpp"
#include "draft/math/glm.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    class EditorApplication;

    /**
     * @brief Displays the loaded project's SubApplication output (rendered into its own
     * offscreen Framebuffer, sharing the editor's GL context) as an ImGui::Image, resizing that
     * Framebuffer to follow the panel's own content region.
     */
    class ViewportPanelSystem : public AbstractSystem {
    public:
        explicit ViewportPanelSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;
        bool on_event(const Event& event) override;

        DRAFT_REFLECTABLE(ViewportPanelSystem)

    private:
        EditorApplication& m_app;
        Vector2d m_regionAvailable{};
        Vector2d m_regionCusorPosition{};
        Vector2d m_regionScreenPosition{};
        bool m_regionHovered = false;

        Vector2d m_regionAvailableLast{};
        bool m_regionHoveredLast = false;
        bool m_regionFocusedLast = false;
    };
}
