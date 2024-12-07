#pragma once

#include "draft/core/registry.hpp"

namespace Draft {
    class AudioSystem {
    private:
        // Variables
        Registry& registryRef;

    public:
        // Public vars
        float dopplerSensitivity = 0.2f;

        // Constructors
        AudioSystem(Registry& registryRef);
        ~AudioSystem() = default;

        // Functions
        void update();
    };
};