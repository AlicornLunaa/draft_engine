#pragma once

#include "draft/math/rect.hpp"

namespace Draft {
    /**
     * @brief A `glScissor` rect that nests. `begin()` pushes this clip's `box` as the active
     * scissor, saving whatever clip was active before so `end()` can restore it (or disable
     * `GL_SCISSOR_TEST` entirely if this was the outermost one). Not thread safe.
     */
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
}
