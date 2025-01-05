#pragma once

#include "draft/interface/unit_value.hpp"
#include "draft/interface/edges.hpp"
#include "draft/interface/size.hpp"
#include "draft/util/color.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    namespace UI {
        struct GlobalStyle {
            Vector2<UnitValue> position = {UnitValue::Auto, UnitValue::Auto};
            Size size;
            Edges margins = {0_pixels, 0_pixels, 0_pixels, 0_pixels};
            Edges padding = {0_pixels, 0_pixels, 0_pixels, 0_pixels};
            Color foregroundColor = Color::WHITE;
            Color backgroundColor = Color::BLACK;
            bool visible = true;
        };
    };
};