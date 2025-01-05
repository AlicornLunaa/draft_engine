#pragma once

#include "draft/interface/unit_value.hpp"
#include "draft/math/rect.hpp"

namespace Draft {
    namespace UI {
        typedef Rect<UnitValue> UnitRect;
        
        struct Metrics {
            struct {
                // Concrete phase consists of measuring in percentages and pixels.
                // Fluid phase resizes parents to fit concrete children if needed
                UnitRect outer = UnitRect(true, true, true, true);
                UnitRect inner = UnitRect(true, true, true, true);
            } fluid;

            struct {
                // Constraint phase resizes elements to fit strict min/max sizes
                // Render phase is the final output
                FloatRect outer{};
                FloatRect inner{};
                Vector4f padding{};
            } constraint;
        };
    };
};