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
     * that Default-layer Image submission.
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
        void draw_translate_handle(Entity entity, const TransformComponent& transform, const Camera& camera);
        void draw_rotate_handle(Entity entity, const TransformComponent& transform, const Camera& camera);

        Vector2f world_to_screen(const Camera& camera, const Vector2f& worldPos) const;
        Vector2f screen_to_world(const Camera& camera, const Vector2f& screenPos) const;

        EditorApplication& m_app;

        // Offset between the rotate handle's initial drag angle and the transform's rotation at
        // that moment, so grabbing the handle doesn't snap the object to point at the cursor.
        float m_dragRotationOffset = 0.f;
    };
}
