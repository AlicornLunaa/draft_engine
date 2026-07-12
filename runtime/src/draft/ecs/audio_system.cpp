#include "draft/ecs/audio_system.hpp"
#include "draft/audio/listener.hpp"
#include "draft/components/audio_components.hpp"
#include "draft/components/rigid_body_component.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/math/glm.hpp"
#include "draft/phys/rigid_body.hpp"

namespace Draft {
    // Constructors
    AudioSystem::AudioSystem(Registry& registryRef) : registryRef(registryRef) {}

    // Functions
    void AudioSystem::render(Time){
        // Update positions of every audio-based component from its entity's transform
        auto listenerView = registryRef.view<ListenerComponent, TransformComponent>();
        auto soundView = registryRef.view<SoundComponent, TransformComponent>();
        auto musicView = registryRef.view<MusicComponent, TransformComponent>();

        // Listeners also picks the highest-priority listener with a body as the doppler observer
        Vector2f observerPosition;
        Vector2f observerVelocity;
        bool hasDopplerListener = false;
        uint maxPriority = 1;

        for(auto ent : listenerView){
            TransformComponent& transform = listenerView.get<TransformComponent>(ent);
            ListenerComponent& listenerComp = listenerView.get<ListenerComponent>(ent);
            Listener& listener = listenerComp;
            listener.set_position(Vector3f(transform.position, listener.get_position().z));
            listener.set_direction(Vector3f(0, 0, -1));
            listener.set_up(Math::rotate(Vector3f(0, 1, 0), transform.rotation, Vector3f(0, 0, 1)));
            listener.apply();

            if(listenerComp.priority >= maxPriority && registryRef.all_of<NativeBodyComponent>(ent)){
                maxPriority = listenerComp.priority;
                hasDopplerListener = true;

                RigidBody& observer = registryRef.get<NativeBodyComponent>(ent);
                observerPosition = observer.get_world_center();
                observerVelocity = observer.get_linear_velocity();
            }
        }

        for(auto ent : soundView){
            TransformComponent& transform = soundView.get<TransformComponent>(ent);
            Sound& sound = soundView.get<SoundComponent>(ent);
            sound.set_position(Vector3f(transform.position, sound.get_position().z));

            if(hasDopplerListener && registryRef.all_of<NativeBodyComponent>(ent)){
                RigidBody& source = registryRef.get<NativeBodyComponent>(ent);
                Vector2f relativeVel = source.get_linear_velocity() - observerVelocity;
                Vector2f direction = source.get_world_center() - observerPosition;
                float dot = Math::dot(relativeVel, direction);
                int sign = -Math::sign(dot);

                float dopplerShift = (343 + Math::length(observerVelocity) * dopplerSensitivity * sign) / (343 - Math::length(source.get_linear_velocity()) * dopplerSensitivity * sign);
                sound.set_pitch(dopplerShift);
            }
        }

        for(auto ent : musicView){
            TransformComponent& transform = musicView.get<TransformComponent>(ent);
            Music* music = musicView.get<MusicComponent>(ent);
            music->set_position(Vector3f(transform.position, music->get_position().z));

            if(hasDopplerListener && registryRef.all_of<NativeBodyComponent>(ent)){
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
}
