#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/asset_manager.hpp"
#include "draft/util/time.hpp"

#include <cstddef>
#include <vector>

namespace Draft {
    struct ParticleProps {
        Vector2f position{0, 0};
        Vector2f velocity{0, 0};
        Vector2f velocityVariation{1, 1};
        Vector4f colorBegin{1, 1, 1, 1};
        Vector4f colorEnd{1, 1, 1, 1};
        float sizeBegin = 0.2f;
        float sizeEnd = 0.1f;
        float sizeVariation = 0.f;
        float lifeTime = 1.f;
        std::shared_ptr<Texture> texture = Assets::manager.get<Texture>("assets/textures/particles/circle.png", true);
    };

    class ParticleSystem {
    private:
        // Structs
        struct Particle {
            ParticleProps props{};
            Vector2f position{0, 0};
            Vector2f velocity{0, 0};
            float rotation = 0.f;
            float size = 0.f;
            float lifeRemaining = 0.f;
            bool active = false;
        };

        // Variables
        std::vector<Particle> particlePool;
        size_t poolIndex;

    public:
        // Constructors
        ParticleSystem(const size_t maxParticles = 1000);

        // Functions
        void emit(const ParticleProps& props);
        void update(Time timeStep);
        void render(SpriteBatch& batch);
    };
};