#include "draft/systems/audio_system.hpp"
#include "draft/components/audio_components.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    // Constructors
    AudioSystem::AudioSystem(Registry& registryRef) : registryRef(registryRef) {
    }

    // Functoins
    void AudioSystem::update(){
        // Update positions of every audio based component
        auto view1 = registryRef.view<ListenerComponent, TransformComponent>();
        auto view2 = registryRef.view<SoundComponent, TransformComponent>();
        auto view3 = registryRef.view<MusicComponent, TransformComponent>();

        // Listeners
        for(auto ent : view1){
            TransformComponent& transform = view1.get<TransformComponent>(ent);
            Listener& listener = view1.get<ListenerComponent>(ent);
            listener.set_position(Vector3f(transform.position, listener.get_position().z));
            listener.set_direction(Vector3f(0, 0, -1));
            listener.set_up(Math::rotate(Vector3f(0, 1, 0), transform.rotation, Vector3f(0, 0, 1)));
            listener.apply();
        }

        // Sounds
        for(auto ent : view2){
            TransformComponent& transform = view2.get<TransformComponent>(ent);
            Sound& sound = view2.get<SoundComponent>(ent);
            sound.set_position(Vector3f(transform.position, sound.get_position().z));
        }

        // Music
        for(auto ent : view3){
            TransformComponent& transform = view2.get<TransformComponent>(ent);
            Music* music = view3.get<MusicComponent>(ent);
            music->set_position(Vector3f(transform.position, music->get_position().z));
        }
    }
};