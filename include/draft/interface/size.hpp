#pragma once

#include "draft/interface/unit_value.hpp"

namespace Draft {
    namespace UI {
        struct Size {
            UnitValue width = UnitValue::Auto;
            UnitValue minWidth = 0_pixels;
            UnitValue maxWidth = 100_percent;

            UnitValue height = UnitValue::Auto;
            UnitValue minHeight = 0_pixels;
            UnitValue maxHeight = 100_percent;
        };
    };
};