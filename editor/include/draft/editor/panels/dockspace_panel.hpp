#pragma once

#include "draft/ecs/system.hpp"
#include "draft/util/reflectable.hpp"

#include <array>

namespace Draft {
    class EditorApplication;

    /**
     * @brief Draws the editor's dockspace, main menu bar, and play controls. Holds no state of
     * its own beyond its own widgets, everything it acts on lives on EditorApplication.
     */
    class DockspacePanelSystem : public AbstractSystem {
    public:
        explicit DockspacePanelSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(DockspacePanelSystem)

    private:
        void draw_menu_bar();
        void draw_play_controls();
        void build_initial_layout(unsigned int dockspaceId);

        EditorApplication& m_app;
        std::array<char, 512> m_projectPathBuffer{};
        bool m_dockspaceBuilt = false;
    };
}
