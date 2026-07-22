#pragma once

#include "draft/ecs/system.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    class EditorApplication;
    struct GizmoViewport;

    /**
     * @brief Draws read-only gizmos (anchor points, angle/translation limits, motor direction,
     * pulley ground anchors, mouse joint target) for whichever joint component the primary
     * selection has, into the same "Viewport" window ColliderGizmoSystem/GizmoOverlaySystem draw
     * into.
     */
    class JointGizmoSystem : public AbstractSystem {
    public:
        explicit JointGizmoSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;
        RenderLayer get_render_layers() const override;

        DRAFT_REFLECTABLE(JointGizmoSystem)

    private:
        EditorApplication& m_app;
    };
}
