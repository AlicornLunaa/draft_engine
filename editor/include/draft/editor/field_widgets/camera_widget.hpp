#pragma once

#include "draft/editor/field_widgets.hpp"
#include "draft/rendering/camera.hpp"

namespace Draft {
    /**
     * @brief Custom inspector widget for Camera, a projection-type dropdown plus the fields of
     * whichever of PerspectiveCameraParams/OrthographicCameraParams is currently active. Camera
     * isn't Reflectable (its params are a std::variant, not a plain aggregate), so generic field
     * recursion can't handle it. This is the "overridable per-type widget" example
     * HasComponentWidget (see field_widgets.hpp) is for. Found by ADL, no registration needed
     * beyond field_widgets.cpp including this header.
     */
    bool draw_component_widget(FieldContext& ctx, std::string_view label, Camera& camera);
}
