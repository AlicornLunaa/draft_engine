#pragma once

#include "draft/interface/unit_value.hpp"

namespace Draft {
    namespace UI {
        struct Edges {
            UnitValue left = UnitValue::Auto;
            UnitValue top = UnitValue::Auto;
            UnitValue right = UnitValue::Auto;
            UnitValue bottom = UnitValue::Auto;
        };
    };
};