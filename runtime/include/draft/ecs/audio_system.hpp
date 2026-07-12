#pragma once

#include "draft/ecs/registry.hpp"
#include "draft/ecs/system.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    /**
     * @brief Keeps Listener/Sound/Music positions in sync with each entity's TransformComponent
     * every frame, and applies whichever ListenerComponent is currently active.
     */
    class AudioSystem : public AbstractSystem {
    private:
        // Variables
        Registry& registryRef;

    public:
        // Public vars
        float dopplerSensitivity = 0.2f;

        // Constructors
        AudioSystem(Registry& registryRef);
        ~AudioSystem() override = default;

        // Functions
        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(AudioSystem, dopplerSensitivity)
    };
}
