#pragma once

#include "draft/math/rect.hpp"

namespace Draft {
    class Clip {
    public:
        // Variables
        FloatRect box;

        // Functions
        void begin();
        void end();
    };
};