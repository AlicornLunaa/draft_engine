#pragma once

#include "draft/ecs/system.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    class EditorApplication;

    /**
     * @brief Lets the user toggle which of the catalog's known system types are attached to
     * gameScene, the same catalog-driven way the Inspector's Add Component menu works for
     * components. Closed by default, see EditorApplication::systemsPanelVisible.
     */
    class SystemsPanelSystem : public AbstractSystem {
    public:
        explicit SystemsPanelSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(SystemsPanelSystem)

    private:
        EditorApplication& m_app;
    };
}
