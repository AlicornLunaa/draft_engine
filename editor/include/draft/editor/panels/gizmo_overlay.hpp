#pragma once

#include "draft/ecs/entity.hpp"
#include "draft/ecs/system.hpp"
#include "draft/math/glm.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    class EditorApplication;
    class Camera;
    struct TransformComponent;

    /**
     * @brief Draws translate/rotate handles for the selected entity's TransformComponent, into
     * the same "Viewport" window ViewportPanelSystem draws (Overlay layer, so this runs after
     * that Default-layer Image submission - ImGui resolves overlapping click targets by
     * submission order, so a click landing on a handle is claimed here rather than falling
     * through to EntityPickerSystem's click-to-select). Edit-mode only, same as
     * FreecamControllerSystem - dragging a gizmo while the simulation is actively ticking would
     * fight with physics writing to the same fields every step.
     *
     * Writes go through Entity::modify_component() rather than a raw reference, so
     * PhysicsSystem's on_update<TransformComponent> hook fires and keeps any attached Collider in
     * sync - unlike a raw field write (e.g. the Inspector's own Vector2f widget).
     */
    class GizmoOverlaySystem : public AbstractSystem {
    public:
        explicit GizmoOverlaySystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;
        RenderLayer get_render_layers() const override;

        DRAFT_REFLECTABLE(GizmoOverlaySystem)

    private:
        bool draw_center_handle(Entity entity, const TransformComponent& transform, const Camera& camera);
        bool draw_axis_handle(Entity entity, const TransformComponent& transform, const Camera& camera, const Vector2f& axisDir, bool isXAxis);
        bool draw_rotate_handle(Entity entity, const TransformComponent& transform, const Camera& camera, const Vector2f& xAxisDir, const Vector2f& yAxisDir);

        Vector2f world_to_screen(const Camera& camera, const Vector2f& worldPos) const;
        Vector2f screen_to_world(const Camera& camera, const Vector2f& screenPos) const;

        EditorApplication& m_app;

        float m_dragRotationOffset = 0.f;
        Vector2f m_dragPositionStart{};
        Vector2f m_dragMouseWorldStart{};
    };
}
