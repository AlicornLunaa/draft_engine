#pragma once

#include "draft/editor/field_widgets.hpp"
#include "draft/physics/collider.hpp"

namespace Draft {
    /**
     * @brief Custom inspector widget for Collider.
     */
    bool draw_component_widget(FieldContext& ctx, std::string_view label, Collider& collider);
}
