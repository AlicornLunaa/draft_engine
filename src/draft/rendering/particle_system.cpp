#include "draft/rendering/particle_system.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"

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
        
        poolIndex = --poolIndex % particlePool.size();
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

    void ParticleSystem::render(SpriteBatch& batch){
        for(auto& particle : particlePool){
            if(!particle.active)
                continue;

            float life = particle.lifeRemaining / particle.props.lifeTime;
            float size = std::lerp(particle.size, particle.props.sizeEnd, life);

            Vector4f color = particle.props.colorBegin + (particle.props.colorEnd - particle.props.colorBegin) * life;
            color.a *= life;

            batch.draw({
                particle.props.texture,
                FloatRect{},
                particle.position,
                particle.rotation,
                Vector2f{particle.size},
                Vector2f{particle.size * 0.5f},
                2.f,
                color
            });
        }
    }
};