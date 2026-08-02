#pragma once

#include "draft/ecs/field_context.hpp"

namespace Draft {
    struct GizmoViewport;

    /**
     * @brief Everything a component's draw_gizmo might need beyond FieldContext: the
     * world/screen conversion for the viewport it's drawing into.
     */
    struct GizmoContext {
        FieldContext& field;
        const GizmoViewport& viewport;
    };
}
