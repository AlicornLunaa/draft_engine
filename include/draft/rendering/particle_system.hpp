#pragma once

#include "draft/rendering/shader.hpp"
#include "draft/rendering/sprite_batch.hpp"
#include "draft/util/asset_manager.hpp"
#include <vector>

namespace Draft {
    struct Particle {

    };

    class ParticleSystem {
    private:
        // Variables
        SpriteBatch batch;
        std::vector<Particle> particles;

    public:
        // Constructors
        ParticleSystem(const Shader& shader = Assets::get_asset<Shader>("./assets/shaders/default"));

        // Functions
        
    };
};