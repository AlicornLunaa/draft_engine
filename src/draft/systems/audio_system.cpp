#include "draft/systems/audio_system.hpp"
#include "draft/audio/listener.hpp"
#include "draft/components/audio_components.hpp"
#include "draft/components/rigid_body_component.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/math/glm.hpp"
#include "draft/phys/rigid_body.hpp"
#include "glm/common.hpp"
#include "glm/geometric.hpp"

namespace Draft {
    // Constructors
    AudioSystem::AudioSystem(Registry& registryRef) : registryRef(registryRef) {
    }

    // Functions
    void AudioSystem::update(){
        // Update positions of every audio based component
        auto view1 = registryRef.view<ListenerComponent, TransformComponent>();
        auto view2 = registryRef.view<SoundComponent, TransformComponent>();
        auto view3 = registryRef.view<MusicComponent, TransformComponent>();

        // Listeners
        Listener* dopplerListener = nullptr;
        Vector2f observerPosition;
        Vector2f observerVelocity;
        uint maxPriority = 1;

        for(auto ent : view1){
            TransformComponent& transform = view1.get<TransformComponent>(ent);
            ListenerComponent& listenerComp = view1.get<ListenerComponent>(ent);
            Listener& listener = listenerComp;
            listener.set_position(Vector3f(transform.position, listener.get_position().z));
            listener.set_direction(Vector3f(0, 0, -1));
            listener.set_up(Math::rotate(Vector3f(0, 1, 0), transform.rotation, Vector3f(0, 0, 1)));
            listener.apply();

            if(listenerComp.priority >= maxPriority && registryRef.all_of<NativeBodyComponent>(ent)){
                maxPriority = listenerComp.priority;
                dopplerListener = &listener;

                RigidBody& observer = registryRef.get<NativeBodyComponent>(ent);
                observerPosition = observer.get_world_center();
                observerVelocity = observer.get_linear_velocity();
            }
        }

        // Sounds
        for(auto ent : view2){
            TransformComponent& transform = view2.get<TransformComponent>(ent);
            Sound& sound = view2.get<SoundComponent>(ent);
            sound.set_position(Vector3f(transform.position, sound.get_position().z));

            // Set sound if doppler exists
            if(dopplerListener && registryRef.all_of<NativeBodyComponent>(ent)){
                RigidBody& source = registryRef.get<NativeBodyComponent>(ent);
                Vector2f relativeVel = source.get_linear_velocity() - observerVelocity;
                Vector2f direction = source.get_world_center() - observerPosition;
                float dot = Math::dot(relativeVel, direction);
                int sign = -Math::sign(dot);

                float dopplerShift = (343 + Math::length(observerVelocity) * dopplerSensitivity * sign) / (343 - Math::length(source.get_linear_velocity()) * dopplerSensitivity * sign);
                sound.set_pitch(dopplerShift);
            }
        }

        // Music
        for(auto ent : view3){
            TransformComponent& transform = view2.get<TransformComponent>(ent);
            Music* music = view3.get<MusicComponent>(ent);
            music->set_position(Vector3f(transform.position, music->get_position().z));

            // Set sound if doppler exists
            if(dopplerListener && registryRef.all_of<NativeBodyComponent>(ent)){
                RigidBody& source = registryRef.get<NativeBodyComponent>(ent);
                Vector2f relativeVel = source.get_linear_velocity() - observerVelocity;
                Vector2f direction = source.get_world_center() - observerPosition;
                float dot = Math::dot(relativeVel, direction);
                int sign = -Math::sign(dot);

                float dopplerShift = (343 + Math::length(observerVelocity) * dopplerSensitivity * sign) / (343 - Math::length(source.get_linear_velocity()) * dopplerSensitivity * sign);
                dopplerShift = Math::max(dopplerShift, 0.f);
                music->set_pitch(dopplerShift);
            }
        }
    }
};