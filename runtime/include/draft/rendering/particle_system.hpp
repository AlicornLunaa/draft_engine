#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/batching/sprite_collection.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/reflectable.hpp"
#include "draft/util/time.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace Draft {
    struct ParticleProps {
        DRAFT_REFLECTED(Vector2f, position) = {0, 0};
        DRAFT_REFLECTED(Vector2f, velocity) = {0, 0};
        DRAFT_REFLECTED(Vector2f, velocityVariation) = {1, 1};
        DRAFT_REFLECTED(Vector4f, colorBegin) = {1, 1, 1, 1};
        DRAFT_REFLECTED(Vector4f, colorEnd) = {1, 1, 1, 1};
        DRAFT_REFLECTED(float, sizeBegin) = 0.2f;
        DRAFT_REFLECTED(float, sizeEnd) = 0.1f;
        DRAFT_REFLECTED(float, sizeVariation) = 0.f;
        DRAFT_REFLECTED(float, lifeTime) = 1.f;
        DRAFT_REFLECTED(std::optional<Resource<Texture>>, texture) = std::nullopt; // None means use SpriteCollection's shared fallback texture
        DRAFT_REFLECTED(std::optional<Resource<Shader>>, shader) = std::nullopt; // None means use SpriteCollection's shared default shader

        DRAFT_REFLECTABLE(ParticleProps, position, velocity, velocityVariation, colorBegin, colorEnd, sizeBegin, sizeEnd, sizeVariation, lifeTime, texture, shader)
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
