#pragma once

#include "draft/ecs/system.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    class EditorApplication;

    /**
     * @brief Editor preferences (snap distances, ...), see EditorSettings. Closed by default,
     * toggled from the dockspace's View menu. Saves on every edit, no separate Save action.
     */
    class SettingsPanelSystem : public AbstractSystem {
    public:
        explicit SettingsPanelSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(SettingsPanelSystem)

    private:
        EditorApplication& m_app;
    };
}
