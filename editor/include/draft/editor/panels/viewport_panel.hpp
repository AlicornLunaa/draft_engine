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

        DRAFT_REFLECTABLE(ViewportPanelSystem)

    private:
        void forward_input(const Vector2d& localPos, bool hovered);

        EditorApplication& m_app;

        // Edge-detection state for remapped mouse buttons/scroll, since gameApp's own hidden
        // window never receives real OS input to detect edges from itself.
        static constexpr int MouseButtonCount = 5;
        bool m_lastButtonDown[MouseButtonCount] = {};
        bool m_wasHovered = false;
    };
}
