#pragma once

#include "draft/ecs/system.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    class EditorApplication;
    class SystemRegistry;
    struct SystemTypeInterface;

    /**
     * @brief Lets the user toggle which of the catalog's known system types are attached to
     * gameScene, the same catalog-driven way the Inspector's Add Component menu works for
     * components. An attached system with at least one reflected field (see
     * SystemTypeInterface::has_fields()) also gets a collapsible "Config" section below its
     * checkbox, editing that system's own fields live the same way the Inspector edits a
     * component's. Closed by default, see EditorApplication::systemsPanelVisible.
     */
    class SystemsPanelSystem : public AbstractSystem {
    public:
        explicit SystemsPanelSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(SystemsPanelSystem)

    private:
        void draw_system_config(SystemTypeInterface& entry, SystemRegistry& registry);

        EditorApplication& m_app;
    };
}
