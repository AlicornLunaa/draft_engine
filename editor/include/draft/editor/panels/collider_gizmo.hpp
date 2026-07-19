#pragma once

#include "draft/ecs/system.hpp"
#include "draft/math/glm.hpp"
#include "draft/util/reflectable.hpp"

#include <functional>

namespace Draft {
    class EditorApplication;
    struct GizmoViewport;
    struct TransformComponent;
    class Collider;
    class Shape;
    class CircleShape;

    /**
     * @brief A shape's points, behind get/set/insert/remove so PolygonShape's index-based API and
     * ChainShape's add()/remove()/clear()-only one can share the same gizmo code below instead of
     * that code needing two copies. insert/remove are left unset (nullptr) for a shape whose
     * point count is fixed (EdgeShape), which the gizmo takes as "this shape can't grow/shrink".
     */
    struct EditablePointList {
        std::function<size_t()> count;
        std::function<Vector2f(size_t)> get;
        std::function<void(size_t, const Vector2f&)> set;
        std::function<void(size_t, const Vector2f&)> insert;
        std::function<void(size_t)> remove;

        // Whether the last point's edge back to the first exists (Polygon, a Chain in LOOP mode)
        // or not (an open Chain, an Edge's single segment).
        bool closed = false;
    };

    /**
     * @brief Draws draggable handles for whichever shape with one handle per point, a midpoint handle per
     * edge that offsets it in/out, and, while holding Ctrl, a ghost point snapped to the nearest
     * edge that inserts a new point there on click. Right-clicking a point deletes it. Shares its
     * handle input/drawing with GizmoOverlaySystem via gizmo_primitives.hpp.
     */
    class ColliderGizmoSystem : public AbstractSystem {
    public:
        explicit ColliderGizmoSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;
        RenderLayer get_render_layers() const override;

        DRAFT_REFLECTABLE(ColliderGizmoSystem)

    private:
        bool draw_point_list_gizmo(EditablePointList& points, const TransformComponent& transform, const GizmoViewport& viewport);
        bool draw_circle_gizmo(CircleShape& circle, const TransformComponent& transform, const GizmoViewport& viewport);

        EditorApplication& m_app;

        // Which point (by index) or edge (by its first point's index) is currently being dragged
        int m_dragPointIndex = -1;
        Vector2f m_dragPointWorldStart{};

        int m_dragEdgeIndex = -1;
        Vector2f m_dragEdgeAWorldStart{};
        Vector2f m_dragEdgeBWorldStart{};
        Vector2f m_dragEdgeNormal{};

        Vector2f m_dragMouseWorldStart{};
    };
}
