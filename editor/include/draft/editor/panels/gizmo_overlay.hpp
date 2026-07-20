#pragma once

#include "draft/ecs/entity.hpp"
#include "draft/ecs/system.hpp"
#include "draft/math/glm.hpp"
#include "draft/util/reflectable.hpp"

#include <vector>

namespace Draft {
    class EditorApplication;
    struct GizmoViewport;
    struct TransformComponent;

    /**
     * @brief Draws translate/rotate handles anchored on the primary selection's
     * TransformComponent, into the same "Viewport" window ViewportPanelSystem draws (Overlay
     * layer, so this runs after that Default-layer Image submission
     *
     * Writes go through Entity::modify_component() rather than a raw reference, so
     * PhysicsSystem's on_update<TransformComponent> hook fires and keeps any attached Collider in
     * sync unlike a raw field write.
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
        bool draw_center_handle(const TransformComponent& transform, const GizmoViewport& viewport);
        bool draw_axis_handle(const TransformComponent& transform, const GizmoViewport& viewport, const Vector2f& axisDir, bool isXAxis);
        bool draw_rotate_handle(const TransformComponent& transform, const GizmoViewport& viewport, const Vector2f& xAxisDir, const Vector2f& yAxisDir);

        void snapshot_drag_group();
        void apply_group_position(const Vector2f& delta);
        void apply_group_rotation(float deltaRotation);

        EditorApplication& m_app;

        float m_dragRotationOffset = 0.f;
        Vector2f m_dragPositionStart{};
        Vector2f m_dragMouseWorldStart{};
        float m_dragRotationStart = 0.f;

        // Snapshot of every selected entity's starting TransformComponent
        std::vector<Entity> m_dragGroup;
        std::vector<Vector2f> m_dragGroupPositionStart;
        std::vector<float> m_dragGroupRotationStart;
    };
}
