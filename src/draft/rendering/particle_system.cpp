#include "draft/rendering/particle_system.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    // Constructors
    ParticleSystem::ParticleSystem(const Shader& shader, const size_t maxParticles) : batch(shader) {
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

    void ParticleSystem::update(float timeStep){
        for(auto& particle : particlePool){
            if(!particle.active)
                continue;

            if(particle.lifeRemaining <= 0.f){
                particle.active = false;
                continue;
            }

            particle.lifeRemaining -= timeStep;
            particle.position += particle.velocity * timeStep;
            particle.rotation += 0.01f * timeStep;
        }
    }

    void ParticleSystem::render(const RenderWindow& window, const Camera* camera){
        if(camera == nullptr)
            return;

        for(auto& particle : particlePool){
            if(!particle.active)
                continue;

            float life = particle.lifeRemaining / particle.props.lifeTime;
            float size = std::lerp(particle.size, particle.props.sizeEnd, life);

            Vector4f color = particle.props.colorBegin + (particle.props.colorEnd - particle.props.colorBegin) * life;
            color.a *= life;

            batch.set_color(color);
            batch.draw(*particle.props.texture, particle.position, {particle.size, particle.size}, particle.rotation, {particle.size / 2.f, particle.size / 2.f});
        }

        camera->apply(window, batch.get_shader());
        batch.flush();
    }
};