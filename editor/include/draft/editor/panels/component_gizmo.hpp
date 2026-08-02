#pragma once

#include "draft/ecs/system.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    class EditorApplication;

    /**
     * @brief Draws draw_gizmo() for every component the primary selection has that defines one.
     */
    class ComponentGizmoSystem : public AbstractSystem {
    public:
        explicit ComponentGizmoSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;
        RenderLayer get_render_layers() const override;

        DRAFT_REFLECTABLE(ComponentGizmoSystem)

    private:
        EditorApplication& m_app;
    };
}
