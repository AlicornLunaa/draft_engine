#include "draft/editor/field_widgets/camera_widget.hpp"

#include "imgui.h"

namespace Draft {
    bool draw_component_widget(FieldContext& ctx, std::string_view label, Camera& camera){
        bool changed = false;
        ImGui::PushID(label.data(), label.data() + label.size());

        Vector3f position = camera.get_position();
        if(ImGui::DragFloat3("Position", &position.x, 0.1f)){
            camera.set_position(position);
            changed = true;
        }

        float rotation = camera.get_rotation();
        if(ImGui::DragFloat("Rotation", &rotation, 0.01f)){
            camera.set_rotation(rotation);
            changed = true;
        }

        static constexpr const char* projectionNames[] = { "Perspective", "Orthographic" };
        int projectionIndex = camera.get_type() == CameraType::PERSPECTIVE ? 0 : 1;

        if(ImGui::Combo("Projection", &projectionIndex, projectionNames, 2)){
            if(projectionIndex == 0)
                camera.set_perspective_params(PerspectiveCameraParams{});
            else
                camera.set_orthographic_params(OrthographicCameraParams{});

            changed = true;
        }

        if(camera.get_type() == CameraType::PERSPECTIVE){
            PerspectiveCameraParams params = camera.get_perspective_params();
            bool paramsChanged = false;

            for_each_field(params, [&](std::string_view name, auto& field){
                paramsChanged |= draw_field(ctx, name, field);
            });

            if(paramsChanged){
                camera.set_perspective_params(params);
                changed = true;
            }
        } else {
            OrthographicCameraParams params = camera.get_orthographic_params();
            bool paramsChanged = false;

            for_each_field(params, [&](std::string_view name, auto& field){
                paramsChanged |= draw_field(ctx, name, field);
            });

            if(paramsChanged){
                camera.set_orthographic_params(params);
                changed = true;
            }
        }

        ImGui::PopID();
        return changed;
    }
}
