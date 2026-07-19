#include "draft/editor/panels/gizmo_overlay.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/panels/entity_picker.hpp"
#include "draft/rendering/camera.hpp"

#include "imgui.h"

namespace Draft {
    namespace {
        constexpr float TRANSLATE_HANDLE_RADIUS = 8.f;
        constexpr float ROTATE_HANDLE_RADIUS = 6.f;
        constexpr float ROTATE_HANDLE_DISTANCE = 40.f;
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
            // Reopens the same window ViewportPanelSystem drew this frame (Default layer,
            // earlier), so these handles land inside it instead of a separate floating window.
            if(ImGui::Begin("Viewport###Viewport")){
                const TransformComponent& transform = selected.get_component<TransformComponent>();
                handledByGizmo |= draw_translate_handle(selected, transform, *camera);
                handledByGizmo |= draw_rotate_handle(selected, transform, *camera);
            }

            ImGui::End();
        }

        // Click-to-select, edit mode only
        if(!handledByGizmo && camera && m_app.gameApp.simulationPaused && m_app.viewportHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
            Vector2f ndc = Vector2f(m_app.gameApp.fakeMouse.get_normalized_position());
            m_app.selection.set(pick_entity(m_app.gameScene, camera->unproject(ndc)));
        }
    }

    bool GizmoOverlaySystem::draw_translate_handle(Entity entity, const TransformComponent& transform, const Camera& camera){
        Vector2f screenPos = world_to_screen(camera, transform.position);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), TRANSLATE_HANDLE_RADIUS, IM_COL32(255, 200, 0, 255));
        drawList->AddCircle(ImVec2(screenPos.x, screenPos.y), TRANSLATE_HANDLE_RADIUS, IM_COL32(0, 0, 0, 255), 0, 2.f);

        ImGui::PushID("GizmoTranslate");
        ImGui::SetCursorScreenPos(ImVec2(screenPos.x - TRANSLATE_HANDLE_RADIUS, screenPos.y - TRANSLATE_HANDLE_RADIUS));
        ImGui::InvisibleButton("##handle", ImVec2(TRANSLATE_HANDLE_RADIUS * 2.f, TRANSLATE_HANDLE_RADIUS * 2.f));

        bool active = ImGui::IsItemActive();

        if(active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
            ImVec2 mouseNow = ImGui::GetMousePos();
            ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
            ImVec2 mousePrev{mouseNow.x - mouseDelta.x, mouseNow.y - mouseDelta.y};

            Vector2f worldNow = screen_to_world(camera, {mouseNow.x, mouseNow.y});
            Vector2f worldPrev = screen_to_world(camera, {mousePrev.x, mousePrev.y});
            Vector2f worldDelta = worldNow - worldPrev;

            entity.modify_component<TransformComponent>([worldDelta](TransformComponent& t){
                t.position += worldDelta;
            });
        }

        ImGui::PopID();
        return active;
    }

    bool GizmoOverlaySystem::draw_rotate_handle(Entity entity, const TransformComponent& transform, const Camera& camera){
        Vector2f handleDirection{Math::sin(transform.rotation), -Math::cos(transform.rotation)};
        Vector2f handleWorldPos = transform.position + handleDirection * ROTATE_HANDLE_DISTANCE;

        Vector2f pivotScreen = world_to_screen(camera, transform.position);
        Vector2f screenPos = world_to_screen(camera, handleWorldPos);

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddLine(ImVec2(pivotScreen.x, pivotScreen.y), ImVec2(screenPos.x, screenPos.y), IM_COL32(0, 200, 255, 255), 2.f);
        drawList->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), ROTATE_HANDLE_RADIUS, IM_COL32(0, 200, 255, 255));

        ImGui::PushID("GizmoRotate");
        ImGui::SetCursorScreenPos(ImVec2(screenPos.x - ROTATE_HANDLE_RADIUS, screenPos.y - ROTATE_HANDLE_RADIUS));
        ImGui::InvisibleButton("##handle", ImVec2(ROTATE_HANDLE_RADIUS * 2.f, ROTATE_HANDLE_RADIUS * 2.f));

        if(ImGui::IsItemActivated()){
            ImVec2 mousePos = ImGui::GetMousePos();
            Vector2f toMouse = screen_to_world(camera, {mousePos.x, mousePos.y}) - transform.position;
            m_dragRotationOffset = transform.rotation - Math::atan(toMouse.y, toMouse.x);
        }

        bool active = ImGui::IsItemActive();

        if(active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
            ImVec2 mousePos = ImGui::GetMousePos();
            Vector2f toMouse = screen_to_world(camera, {mousePos.x, mousePos.y}) - transform.position;
            float newRotation = Math::atan(toMouse.y, toMouse.x) + m_dragRotationOffset;

            entity.modify_component<TransformComponent>([newRotation](TransformComponent& t){
                t.rotation = newRotation;
            });
        }

        ImGui::PopID();
        return active;
    }

    // Matches Mouse::get_normalized_position()'s pixel<->NDC convention (viewport-local, Y
    // flipped), inverted, so a handle drawn here lines up with where a click there would land.
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
