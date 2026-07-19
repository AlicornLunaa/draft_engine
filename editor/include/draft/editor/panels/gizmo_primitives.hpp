#pragma once

#include "draft/math/glm.hpp"

namespace Draft {
    class Camera;

    /**
     * @brief Converts between world space and this frame's Viewport window in screen pixels.
     */
    struct GizmoViewport {
        const Camera& camera;
        Vector2d screenPosition;
        Vector2d size;

        Vector2f world_to_screen(const Vector2f& worldPos) const;
        Vector2f screen_to_world(const Vector2f& screenPos) const;
    };

    /**
     * @brief What happened to one handle this frame. A caller decides what a drag or right click
     * actually does
     */
    struct HandleInteraction {
        bool hovered = false;
        bool active = false; // Held down, whether or not it's moved yet.
        bool justActivated = false; // First frame of this press, the moment to capture drag-start state.
        bool rightClicked = false;
    };

    /**
     * @brief Hit-tests a circular region at @p screenPos in the current ImGui window (must be
     * called between Begin()/End()), drawing nothing. A caller with its own visual (e.g.
     * GizmoOverlaySystem's axis arrows) draws separately but still shares this input handling
     * @param strId Unique (within the current ID stack) id for this handle.
     */
    HandleInteraction hit_test_handle(const char* strId, const Vector2f& screenPos, float radius);

    /**
     * @brief Draws a filled circle at @p screenPos, using @p hoverColor instead of @p color
     * while @p interaction is hovered or active.
     */
    void draw_handle_circle(const Vector2f& screenPos, float radius, const HandleInteraction& interaction, const Vector4f& color, const Vector4f& hoverColor);

    /**
     * @brief hit_test_handle() + draw_handle_circle() together, for the common case of a handle
     * that's just a circle.
     */
    HandleInteraction draw_circle_handle(const char* strId, const Vector2f& screenPos, float radius, const Vector4f& color, const Vector4f& hoverColor);
}
