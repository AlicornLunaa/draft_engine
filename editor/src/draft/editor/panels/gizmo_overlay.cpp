#include "draft/editor/panels/gizmo_overlay.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/panels/entity_picker.hpp"
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

        constexpr float POSITION_SNAP_STEP = 10.f;
        const float ROTATION_SNAP_STEP = Math::radians(15.f);

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
        if(layer != RenderLayer::Interface)
            return;

        Camera* camera = m_app.gameScene.get_active_camera();
        Entity selected = m_app.selection.get();
        bool handledByGizmo = false;

        if(camera && selected.is_valid() && selected.has_component<TransformComponent>()){
            if(ImGui::Begin("Viewport###Viewport")){
                const TransformComponent& transform = selected.get_component<TransformComponent>();
                Vector2f xAxisDir = axis_x(transform.rotation);
                Vector2f yAxisDir = axis_y(transform.rotation);

                handledByGizmo |= draw_axis_handle(selected, transform, *camera, xAxisDir, true);
                handledByGizmo |= draw_axis_handle(selected, transform, *camera, yAxisDir, false);
                handledByGizmo |= draw_rotate_handle(selected, transform, *camera, xAxisDir, yAxisDir);
                handledByGizmo |= draw_center_handle(selected, transform, *camera);
            }

            ImGui::End();
        }

        if(!handledByGizmo && camera && m_app.gameApp.simulationPaused && m_app.viewportHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
            Vector2f ndc = Vector2f(m_app.gameApp.fakeMouse.get_normalized_position());
            m_app.selection.set(pick_entity(m_app.gameScene, camera->unproject(ndc)));
        }
    }

    bool GizmoOverlaySystem::draw_center_handle(Entity entity, const TransformComponent& transform, const Camera& camera){
        Vector2f screenPos = world_to_screen(camera, transform.position);

        ImGui::PushID("GizmoCenter");
        ImGui::SetCursorScreenPos(ImVec2(screenPos.x - CENTER_HANDLE_RADIUS, screenPos.y - CENTER_HANDLE_RADIUS));
        ImGui::InvisibleButton("##handle", ImVec2(CENTER_HANDLE_RADIUS * 2.f, CENTER_HANDLE_RADIUS * 2.f));

        bool hovered = ImGui::IsItemHovered();

        if(ImGui::IsItemActivated()){
            ImVec2 mousePos = ImGui::GetMousePos();
            m_dragPositionStart = transform.position;
            m_dragMouseWorldStart = screen_to_world(camera, {mousePos.x, mousePos.y});
        }

        bool active = ImGui::IsItemActive();
        ImU32 color = (hovered || active) ? IM_COL32(255, 230, 120, 255) : IM_COL32(255, 200, 0, 255);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), CENTER_HANDLE_RADIUS, color);
        drawList->AddCircle(ImVec2(screenPos.x, screenPos.y), CENTER_HANDLE_RADIUS, IM_COL32(0, 0, 0, 255), 0, 2.f);

        if(active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
            ImVec2 mousePos = ImGui::GetMousePos();
            Vector2f mouseWorldNow = screen_to_world(camera, {mousePos.x, mousePos.y});
            Vector2f newPosition = m_dragPositionStart + (mouseWorldNow - m_dragMouseWorldStart);

            if(ImGui::GetIO().KeyShift)
                newPosition = { snap_to_step(newPosition.x, POSITION_SNAP_STEP), snap_to_step(newPosition.y, POSITION_SNAP_STEP) };

            entity.modify_component<TransformComponent>([newPosition](TransformComponent& t){
                t.position = newPosition;
            });
        }

        ImGui::PopID();
        return active;
    }

    bool GizmoOverlaySystem::draw_axis_handle(Entity entity, const TransformComponent& transform, const Camera& camera, const Vector2f& axisDir, bool isXAxis){
        Vector2f tipWorld = transform.position + axisDir * AXIS_LENGTH;
        Vector2f pivotScreen = world_to_screen(camera, transform.position);
        Vector2f tipScreen = world_to_screen(camera, tipWorld);

        ImGui::PushID(isXAxis ? "GizmoAxisX" : "GizmoAxisY");
        ImGui::SetCursorScreenPos(ImVec2(tipScreen.x - AXIS_HANDLE_RADIUS, tipScreen.y - AXIS_HANDLE_RADIUS));
        ImGui::InvisibleButton("##handle", ImVec2(AXIS_HANDLE_RADIUS * 2.f, AXIS_HANDLE_RADIUS * 2.f));

        bool hovered = ImGui::IsItemHovered();

        if(ImGui::IsItemActivated()){
            ImVec2 mousePos = ImGui::GetMousePos();
            m_dragPositionStart = transform.position;
            m_dragMouseWorldStart = screen_to_world(camera, {mousePos.x, mousePos.y});
        }

        bool active = ImGui::IsItemActive();
        ImU32 normalColor = isXAxis ? IM_COL32(220, 60, 60, 255) : IM_COL32(60, 200, 90, 255);
        ImU32 hoverColor = isXAxis ? IM_COL32(255, 130, 130, 255) : IM_COL32(150, 255, 170, 255);
        ImU32 color = (hovered || active) ? hoverColor : normalColor;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddLine(ImVec2(pivotScreen.x, pivotScreen.y), ImVec2(tipScreen.x, tipScreen.y), color, 3.f);
        draw_arrowhead(drawList, tipScreen, pivotScreen, color);

        if(active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
            ImVec2 mousePos = ImGui::GetMousePos();
            Vector2f mouseWorldNow = screen_to_world(camera, {mousePos.x, mousePos.y});
            float distance = Math::dot(mouseWorldNow - m_dragMouseWorldStart, axisDir);

            if(ImGui::GetIO().KeyShift)
                distance = snap_to_step(distance, POSITION_SNAP_STEP);

            Vector2f newPosition = m_dragPositionStart + axisDir * distance;

            entity.modify_component<TransformComponent>([newPosition](TransformComponent& t){
                t.position = newPosition;
            });
        }

        ImGui::PopID();
        return active;
    }

    bool GizmoOverlaySystem::draw_rotate_handle(Entity entity, const TransformComponent& transform, const Camera& camera, const Vector2f& xAxisDir, const Vector2f& yAxisDir){
        Vector2f xTipWorld = transform.position + xAxisDir * AXIS_LENGTH;
        Vector2f yTipWorld = transform.position + yAxisDir * AXIS_LENGTH;
        Vector2f cornerWorld = xTipWorld + yAxisDir * AXIS_LENGTH;

        Vector2f xTipScreen = world_to_screen(camera, xTipWorld);
        Vector2f yTipScreen = world_to_screen(camera, yTipWorld);
        Vector2f cornerScreen = world_to_screen(camera, cornerWorld);

        ImGui::PushID("GizmoRotate");
        ImGui::SetCursorScreenPos(ImVec2(cornerScreen.x - ROTATE_HANDLE_RADIUS, cornerScreen.y - ROTATE_HANDLE_RADIUS));
        ImGui::InvisibleButton("##handle", ImVec2(ROTATE_HANDLE_RADIUS * 2.f, ROTATE_HANDLE_RADIUS * 2.f));

        bool hovered = ImGui::IsItemHovered();

        if(ImGui::IsItemActivated()){
            ImVec2 mousePos = ImGui::GetMousePos();
            Vector2f toMouse = screen_to_world(camera, {mousePos.x, mousePos.y}) - transform.position;
            m_dragRotationOffset = transform.rotation - Math::atan(toMouse.y, toMouse.x);
        }

        bool active = ImGui::IsItemActive();
        ImU32 color = (hovered || active) ? IM_COL32(150, 230, 255, 255) : IM_COL32(0, 200, 255, 255);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddLine(ImVec2(xTipScreen.x, xTipScreen.y), ImVec2(cornerScreen.x, cornerScreen.y), IM_COL32(200, 200, 200, 150), 1.5f);
        drawList->AddLine(ImVec2(yTipScreen.x, yTipScreen.y), ImVec2(cornerScreen.x, cornerScreen.y), IM_COL32(200, 200, 200, 150), 1.5f);
        drawList->AddRectFilled(ImVec2(cornerScreen.x - ROTATE_HANDLE_RADIUS, cornerScreen.y - ROTATE_HANDLE_RADIUS), ImVec2(cornerScreen.x + ROTATE_HANDLE_RADIUS, cornerScreen.y + ROTATE_HANDLE_RADIUS), color);

        if(active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
            ImVec2 mousePos = ImGui::GetMousePos();
            Vector2f toMouse = screen_to_world(camera, {mousePos.x, mousePos.y}) - transform.position;
            float newRotation = Math::atan(toMouse.y, toMouse.x) + m_dragRotationOffset;

            if(ImGui::GetIO().KeyShift)
                newRotation = snap_to_step(newRotation, ROTATION_SNAP_STEP);

            entity.modify_component<TransformComponent>([newRotation](TransformComponent& t){
                t.rotation = newRotation;
            });
        }

        ImGui::PopID();
        return active;
    }

    Vector2f GizmoOverlaySystem::world_to_screen(const Camera& camera, const Vector2f& worldPos) const {
        Vector2f ndc = camera.project(worldPos);
        Vector2d size = m_app.viewportSize;

        Vector2f local{
            static_cast<float>(size.x * (ndc.x / 2.0 + 0.5)),
            static_cast<float>(size.y * (0.5 - ndc.y / 2.0))
        };

        return local + Vector2f(m_app.viewportScreenPosition);
    }

    Vector2f GizmoOverlaySystem::screen_to_world(const Camera& camera, const Vector2f& screenPos) const {
        Vector2f local = screenPos - Vector2f(m_app.viewportScreenPosition);
        Vector2d size = m_app.viewportSize;

        Vector2f ndc{
            static_cast<float>((local.x / size.x - 0.5) * 2.0),
            static_cast<float>((1.0 - local.y / size.y - 0.5) * 2.0)
        };

        return camera.unproject(ndc);
    }
}
