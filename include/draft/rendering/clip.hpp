#pragma once

#include "draft/math/rect.hpp"

namespace Draft {
    class Clip {
    private:
        // Static data
        static Clip* currentClip;

        // Variables
        Clip* previousClip = nullptr;

        // Private functions
        void cut();

    public:
        // Variables
        FloatRect box;

        // Functions
        void begin();
        void end();
    };
};