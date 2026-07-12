#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/batching/sprite_collection.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
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
        Texture* texture = nullptr; // Nullptr means use SpriteCollection's shared fallback texture
        Shader* shader = nullptr; // Nullptr means use SpriteCollection's shared default shader
    };

    /**
     * @brief Fixed-size, ring-buffer-recycled CPU particle pool
     */
    class ParticleSystem {
    public:
        /**
         * @brief The interpolated size + tint for a particle at a given point in its life.
         */
        struct VisualState {
            float size;
            Vector4f color;
        };

        // Constructors
        ParticleSystem(const size_t maxParticles = 1000);

        // Functions
        void emit(const ParticleProps& props);
        void update(Time timeStep);
        void render(SpriteCollection& batch);

    private:
        friend struct ParticleSystemTestAccess;

        /**
         * @brief Computes a particle's size/color at a given point in its life.
         */
        static VisualState compute_visual_state(float startSize, float endSize, const Vector4f& colorBegin, const Vector4f& colorEnd, float lifeFraction);

        /**
         * @brief Decrements a pool index with wraparound (0 wraps to `poolSize - 1`)
         */
        static size_t recycle_index(size_t poolIndex, size_t poolSize);

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
    };
}
