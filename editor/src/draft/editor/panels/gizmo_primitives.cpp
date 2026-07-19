#include "draft/editor/panels/gizmo_primitives.hpp"
#include "draft/rendering/camera.hpp"

#include "imgui.h"

namespace Draft {
    // Matches Mouse::get_normalized_position()'s pixel<->NDC convention (viewport-local, Y
    // flipped), so a handle drawn here lines up with where a click there would land.
    Vector2f GizmoViewport::world_to_screen(const Vector2f& worldPos) const {
        Vector2f ndc = camera.project(worldPos);

        Vector2f local{
            static_cast<float>(size.x * (ndc.x / 2.0 + 0.5)),
            static_cast<float>(size.y * (0.5 - ndc.y / 2.0))
        };

        return local + Vector2f(screenPosition);
    }

    Vector2f GizmoViewport::screen_to_world(const Vector2f& screenPos) const {
        Vector2f local = screenPos - Vector2f(screenPosition);

        Vector2f ndc{
            static_cast<float>((local.x / size.x - 0.5) * 2.0),
            static_cast<float>((1.0 - local.y / size.y - 0.5) * 2.0)
        };

        return camera.unproject(ndc);
    }

    HandleInteraction hit_test_handle(const char* strId, const Vector2f& screenPos, float radius){
        ImGui::PushID(strId);
        ImGui::SetCursorScreenPos(ImVec2(screenPos.x - radius, screenPos.y - radius));
        ImGui::InvisibleButton("##handle", ImVec2(radius * 2.f, radius * 2.f));

        HandleInteraction interaction;
        interaction.hovered = ImGui::IsItemHovered();
        interaction.active = ImGui::IsItemActive();
        interaction.justActivated = ImGui::IsItemActivated();
        interaction.rightClicked = interaction.hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right);

        ImGui::PopID();
        return interaction;
    }

    void draw_handle_circle(const Vector2f& screenPos, float radius, const HandleInteraction& interaction, const Vector4f& color, const Vector4f& hoverColor){
        const Vector4f& drawColor = (interaction.hovered || interaction.active) ? hoverColor : color;
        ImU32 packedColor = ImGui::ColorConvertFloat4ToU32(ImVec4(drawColor.r, drawColor.g, drawColor.b, drawColor.a));

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->AddCircleFilled(ImVec2(screenPos.x, screenPos.y), radius, packedColor);
        drawList->AddCircle(ImVec2(screenPos.x, screenPos.y), radius, IM_COL32(0, 0, 0, 255), 0, 2.f);
    }

    HandleInteraction draw_circle_handle(const char* strId, const Vector2f& screenPos, float radius, const Vector4f& color, const Vector4f& hoverColor){
        HandleInteraction interaction = hit_test_handle(strId, screenPos, radius);
        draw_handle_circle(screenPos, radius, interaction, color, hoverColor);
        return interaction;
    }
}
