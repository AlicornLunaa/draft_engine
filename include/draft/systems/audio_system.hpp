#pragma once

#include "draft/core/registry.hpp"

namespace Draft {
    class AudioSystem {
    private:
        // Variables
        Registry& registryRef;

    public:
        // Constructors
        AudioSystem(Registry& registryRef);
        ~AudioSystem() = default;

        // Functions
        void update();
    };
};