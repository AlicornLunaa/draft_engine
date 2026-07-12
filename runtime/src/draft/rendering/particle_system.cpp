#include "draft/rendering/particle_system.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/material.hpp"

#include <cstdlib>

namespace Draft {
    // Constructors
    ParticleSystem::ParticleSystem(const size_t maxParticles) {
        particlePool.resize(maxParticles);
        poolIndex = maxParticles - 1;
    }

    // Functions
    void ParticleSystem::emit(const ParticleProps& props){
        Particle& particle = particlePool[poolIndex];

        particle.props = props;
        particle.position = props.position;
        particle.rotation = (std::rand() / (float)RAND_MAX) * 2.f * 3.14f;
        particle.active = true;

        particle.velocity = props.velocity;
        particle.velocity += props.velocityVariation * Vector2f(std::rand() / (float)RAND_MAX - 0.5f, std::rand() / (float)RAND_MAX - 0.5f);

        particle.lifeRemaining = props.lifeTime;
        particle.size = props.sizeBegin + props.sizeVariation * (std::rand() / (float)RAND_MAX - 0.5f);

        poolIndex = recycle_index(poolIndex, particlePool.size());
    }

    size_t ParticleSystem::recycle_index(size_t poolIndex, size_t poolSize){
        return (poolIndex == 0) ? (poolSize - 1) : (poolIndex - 1);
    }

    void ParticleSystem::update(Time timeStep){
        for(auto& particle : particlePool){
            if(!particle.active)
                continue;

            if(particle.lifeRemaining <= 0.f){
                particle.active = false;
                continue;
            }

            particle.lifeRemaining -= timeStep.as_seconds();
            particle.position += particle.velocity * timeStep.as_seconds();
            particle.rotation += 0.01f * timeStep.as_seconds();
        }
    }

    ParticleSystem::VisualState ParticleSystem::compute_visual_state(float startSize, float endSize, const Vector4f& colorBegin, const Vector4f& colorEnd, float lifeFraction){
        float age = 1.f - lifeFraction;

        VisualState state;
        state.size = std::lerp(startSize, endSize, age);
        state.color = colorBegin + (colorEnd - colorBegin) * age;
        state.color.a *= lifeFraction;

        return state;
    }

    void ParticleSystem::render(SpriteCollection& batch){
        for(auto& particle : particlePool){
            if(!particle.active)
                continue;

            float lifeFraction = particle.lifeRemaining / particle.props.lifeTime;
            VisualState visual = compute_visual_state(particle.size, particle.props.sizeEnd, particle.props.colorBegin, particle.props.colorEnd, lifeFraction);

            // SpriteCollection::draw() already falls back to its own shared texture/shader
            // defaults whenever baseTexture/shader are left null
            Material2D particleMaterial;
            particleMaterial.baseTexture = particle.props.texture ? particle.props.texture->get() : nullptr;
            particleMaterial.shader = particle.props.shader ? particle.props.shader->get() : nullptr;
            particleMaterial.tint = visual.color;

            batch.draw(SpriteProps{
                particle.position,
                particle.rotation,
                Vector2f{visual.size},
                Vector2f{visual.size * 0.5f},
                2.f,
                {},
                particleMaterial
            });
        }
    }
}
