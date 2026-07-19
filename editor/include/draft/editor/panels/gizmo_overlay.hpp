#pragma once

#include "draft/ecs/entity.hpp"
#include "draft/ecs/system.hpp"
#include "draft/math/glm.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    class EditorApplication;
    struct GizmoViewport;
    struct TransformComponent;

    /**
     * @brief Draws translate/rotate handles for the selected entity's TransformComponent, into
     * the same "Viewport" window ViewportPanelSystem draws (Overlay layer, so this runs after
     * that Default-layer Image submission - ImGui resolves overlapping click targets by
     * submission order, so a click landing on a handle is claimed here rather than falling
     * through to EntityPickerSystem's click-to-select).
     *
     * Writes go through Entity::modify_component() rather than a raw reference, so
     * PhysicsSystem's on_update<TransformComponent> hook fires and keeps any attached Collider in
     * sync - unlike a raw field write (e.g. the Inspector's own Vector2f widget).
     *
     * Handle input/visuals (hit-testing a circle, hover highlight, world<->screen conversion)
     * come from gizmo_primitives.hpp, shared with ColliderGizmoSystem's vertex handles.
     */
    class GizmoOverlaySystem : public AbstractSystem {
    public:
        explicit GizmoOverlaySystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;
        RenderLayer get_render_layers() const override;

        DRAFT_REFLECTABLE(GizmoOverlaySystem)

    private:
        bool draw_center_handle(Entity entity, const TransformComponent& transform, const GizmoViewport& viewport);
        bool draw_axis_handle(Entity entity, const TransformComponent& transform, const GizmoViewport& viewport, const Vector2f& axisDir, bool isXAxis);
        bool draw_rotate_handle(Entity entity, const TransformComponent& transform, const GizmoViewport& viewport, const Vector2f& xAxisDir, const Vector2f& yAxisDir);

        EditorApplication& m_app;

        float m_dragRotationOffset = 0.f;
        Vector2f m_dragPositionStart{};
        Vector2f m_dragMouseWorldStart{};
    };
}
