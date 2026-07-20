#include "draft/editor/panels/gizmo_overlay.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/panels/entity_picker.hpp"
#include "draft/editor/panels/gizmo_primitives.hpp"
#include "draft/rendering/camera.hpp"

#include "imgui.h"

namespace Draft {
    namespace {
        constexpr float CENTER_HANDLE_RADIUS = 8.f;
        constexpr float AXIS_HANDLE_RADIUS = 7.f;
        constexpr float ROTATE_HANDLE_RADIUS = 6.f;
        constexpr float AXIS_LENGTH = 40.f;
        constexpr float ARROWHEAD_LENGTH = 10.f;
        constexpr float ARROWHEAD_WIDTH = 6.f;

        constexpr Vector4f CENTER_COLOR{1.f, 0.784f, 0.f, 1.f};
        constexpr Vector4f CENTER_HOVER_COLOR{1.f, 0.902f, 0.47f, 1.f};
        constexpr Vector4f X_AXIS_COLOR{0.863f, 0.235f, 0.235f, 1.f};
        constexpr Vector4f X_AXIS_HOVER_COLOR{1.f, 0.51f, 0.51f, 1.f};
        constexpr Vector4f Y_AXIS_COLOR{0.235f, 0.784f, 0.353f, 1.f};
        constexpr Vector4f Y_AXIS_HOVER_COLOR{0.588f, 1.f, 0.667f, 1.f};
        constexpr Vector4f ROTATE_COLOR{0.f, 0.784f, 1.f, 1.f};
        constexpr Vector4f ROTATE_HOVER_COLOR{0.588f, 0.902f, 1.f, 1.f};

        float snap_to_step(float value, float step){
            return Math::round(value / step) * step;
        }

        Vector2f axis_x(float rotation){ return {Math::cos(rotation), Math::sin(rotation)}; }
        Vector2f axis_y(float rotation){ return {Math::sin(rotation), -Math::cos(rotation)}; }

        void draw_arrowhead(ImDrawList* drawList, const Vector2f& tipScreen, const Vector2f& fromScreen, ImU32 color){
            Vector2f dir = Math::normalize(tipScreen - fromScreen);
            Vector2f perp{-dir.y, dir.x};
            Vector2f base = tipScreen - dir * ARROWHEAD_LENGTH;
            Vector2f left = base + perp * ARROWHEAD_WIDTH;
            Vector2f right = base - perp * ARROWHEAD_WIDTH;

            drawList->AddTriangleFilled(ImVec2(tipScreen.x, tipScreen.y), ImVec2(left.x, left.y), ImVec2(right.x, right.y), color);
        }
    }

    GizmoOverlaySystem::GizmoOverlaySystem(EditorApplication& app) : m_app(app) {}

    RenderLayer GizmoOverlaySystem::get_render_layers() const { return RenderLayer::Interface; }

    void GizmoOverlaySystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Interface || !m_app.viewportPanelVisible)
            return;

        Camera* camera = m_app.gameScene.get_active_camera();
        Entity selected = m_app.selection.get();
        bool handledByGizmo = false;

        if(camera && selected.is_valid() && selected.has_component<TransformComponent>()){
            GizmoViewport viewport{*camera, m_app.viewportScreenPosition, m_app.viewportSize};

            if(ImGui::Begin("Viewport###Viewport")){
                const TransformComponent& transform = selected.get_component<TransformComponent>();
                Vector2f xAxisDir = axis_x(transform.rotation);
                Vector2f yAxisDir = axis_y(transform.rotation);

                handledByGizmo |= draw_axis_handle(selected, transform, viewport, xAxisDir, true);
                handledByGizmo |= draw_axis_handle(selected, transform, viewport, yAxisDir, false);
                handledByGizmo |= draw_rotate_handle(selected, transform, viewport, xAxisDir, yAxisDir);
                handledByGizmo |= draw_center_handle(selected, transform, viewport);
            }

            ImGui::End();
        }

        if(!handledByGizmo && !m_app.colliderGizmoActiveThisFrame && camera && m_app.gameApp.simulationPaused && m_app.viewportHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
            Vector2f ndc = Vector2f(m_app.gameApp.fakeMouse.get_normalized_position());
            m_app.selection.set(pick_entity(m_app.gameScene, camera->unproject(ndc)));
        }
    }

    bool GizmoOverlaySystem::draw_center_handle(Entity entity, const TransformComponent& transform, const GizmoViewport& viewport){
        // Hit-test against last frame's known position. Once a drag is already active, ImGui doesn't require the cursor to stay over
        // that rect to keep going, so this doesn't need to be frame-accurate for that part.
        Vector2f screenPos = viewport.world_to_screen(transform.position);
        HandleInteraction interaction = hit_test_handle("GizmoCenter", screenPos, CENTER_HANDLE_RADIUS);

        if(interaction.justActivated){
            ImVec2 mousePos = ImGui::GetMousePos();
            m_dragPositionStart = transform.position;
            m_dragMouseWorldStart = viewport.screen_to_world({mousePos.x, mousePos.y});
        }

        if(interaction.active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
            ImVec2 mousePos = ImGui::GetMousePos();
            Vector2f mouseWorldNow = viewport.screen_to_world({mousePos.x, mousePos.y});
            Vector2f newPosition = m_dragPositionStart + (mouseWorldNow - m_dragMouseWorldStart);

            if(ImGui::GetIO().KeyShift){
                float step = m_app.settings.positionSnapStep;
                newPosition = { snap_to_step(newPosition.x, step), snap_to_step(newPosition.y, step) };
            }

            entity.modify_component<TransformComponent>([newPosition](TransformComponent& t){
                t.position = newPosition;
            });

            // transform is a reference into the live component, patched in place above, so
            // re-reading it now reflects this frame's drag
            screenPos = viewport.world_to_screen(transform.position);
        }

        draw_handle_circle(screenPos, CENTER_HANDLE_RADIUS, interaction, CENTER_COLOR, CENTER_HOVER_COLOR);
        return interaction.active;
    }

    bool GizmoOverlaySystem::draw_axis_handle(Entity entity, const TransformComponent& transform, const GizmoViewport& viewport, const Vector2f& axisDir, bool isXAxis){
        Vector2f pivotScreen = viewport.world_to_screen(transform.position);
        Vector2f tipScreen = viewport.world_to_screen(transform.position + axisDir * AXIS_LENGTH);

        HandleInteraction interaction = hit_test_handle(isXAxis ? "GizmoAxisX" : "GizmoAxisY", tipScreen, AXIS_HANDLE_RADIUS);

        if(interaction.justActivated){
            ImVec2 mousePos = ImGui::GetMousePos();
            m_dragPositionStart = transform.position;
            m_dragMouseWorldStart = viewport.screen_to_world({mousePos.x, mousePos.y});
        }

        if(interaction.active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
            ImVec2 mousePos = ImGui::GetMousePos();
            Vector2f mouseWorldNow = viewport.screen_to_world({mousePos.x, mousePos.y});
            float distance = Math::dot(mouseWorldNow - m_dragMouseWorldStart, axisDir);

            if(ImGui::GetIO().KeyShift)
                distance = snap_to_step(distance, m_app.settings.positionSnapStep);

            Vector2f newPosition = m_dragPositionStart + axisDir * distance;

            entity.modify_component<TransformComponent>([newPosition](TransformComponent& t){
                t.position = newPosition;
            });

            pivotScreen = viewport.world_to_screen(transform.position);
            tipScreen = viewport.world_to_screen(transform.position + axisDir * AXIS_LENGTH);
        }

        const Vector4f& color = isXAxis ? X_AXIS_COLOR : Y_AXIS_COLOR;
        const Vector4f& hoverColor = isXAxis ? X_AXIS_HOVER_COLOR : Y_AXIS_HOVER_COLOR;
        const Vector4f& drawColor = (interaction.hovered || interaction.active) ? hoverColor : color;
        ImU32 packedColor = ImGui::ColorConvertFloat4ToU32(ImVec4(drawColor.r, drawColor.g, drawColor.b, drawColor.a));

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddLine(ImVec2(pivotScreen.x, pivotScreen.y), ImVec2(tipScreen.x, tipScreen.y), packedColor, 3.f);
        draw_arrowhead(drawList, tipScreen, pivotScreen, packedColor);

        return interaction.active;
    }

    bool GizmoOverlaySystem::draw_rotate_handle(Entity entity, const TransformComponent& transform, const GizmoViewport& viewport, const Vector2f& xAxisDir, const Vector2f& yAxisDir){
        Vector2f xTipWorld = transform.position + xAxisDir * AXIS_LENGTH;
        Vector2f yTipWorld = transform.position + yAxisDir * AXIS_LENGTH;
        Vector2f cornerWorld = xTipWorld + yAxisDir * AXIS_LENGTH;

        Vector2f xTipScreen = viewport.world_to_screen(xTipWorld);
        Vector2f yTipScreen = viewport.world_to_screen(yTipWorld);
        Vector2f cornerScreen = viewport.world_to_screen(cornerWorld);

        HandleInteraction interaction = hit_test_handle("GizmoRotate", cornerScreen, ROTATE_HANDLE_RADIUS);

        if(interaction.justActivated){
            ImVec2 mousePos = ImGui::GetMousePos();
            Vector2f toMouse = viewport.screen_to_world({mousePos.x, mousePos.y}) - transform.position;
            m_dragRotationOffset = transform.rotation - Math::atan(toMouse.y, toMouse.x);
        }

        if(interaction.active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
            ImVec2 mousePos = ImGui::GetMousePos();
            Vector2f toMouse = viewport.screen_to_world({mousePos.x, mousePos.y}) - transform.position;
            float newRotation = Math::atan(toMouse.y, toMouse.x) + m_dragRotationOffset;

            if(ImGui::GetIO().KeyShift)
                newRotation = snap_to_step(newRotation, Math::radians(m_app.settings.rotationSnapStepDegrees));

            entity.modify_component<TransformComponent>([newRotation](TransformComponent& t){
                t.rotation = newRotation;
            });

            Vector2f freshXAxis = axis_x(transform.rotation);
            Vector2f freshYAxis = axis_y(transform.rotation);
            xTipScreen = viewport.world_to_screen(transform.position + freshXAxis * AXIS_LENGTH);
            yTipScreen = viewport.world_to_screen(transform.position + freshYAxis * AXIS_LENGTH);
            cornerScreen = viewport.world_to_screen(transform.position + freshXAxis * AXIS_LENGTH + freshYAxis * AXIS_LENGTH);
        }

        const Vector4f& drawColor = (interaction.hovered || interaction.active) ? ROTATE_HOVER_COLOR : ROTATE_COLOR;
        ImU32 packedColor = ImGui::ColorConvertFloat4ToU32(ImVec4(drawColor.r, drawColor.g, drawColor.b, drawColor.a));

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddLine(ImVec2(xTipScreen.x, xTipScreen.y), ImVec2(cornerScreen.x, cornerScreen.y), IM_COL32(200, 200, 200, 150), 1.5f);
        drawList->AddLine(ImVec2(yTipScreen.x, yTipScreen.y), ImVec2(cornerScreen.x, cornerScreen.y), IM_COL32(200, 200, 200, 150), 1.5f);
        drawList->AddRectFilled(ImVec2(cornerScreen.x - ROTATE_HANDLE_RADIUS, cornerScreen.y - ROTATE_HANDLE_RADIUS), ImVec2(cornerScreen.x + ROTATE_HANDLE_RADIUS, cornerScreen.y + ROTATE_HANDLE_RADIUS), packedColor);

        return interaction.active;
    }
}
