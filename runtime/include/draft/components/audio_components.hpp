#pragma once

#include "draft/asset/resource.hpp"
#include "draft/audio/listener.hpp"
#include "draft/audio/music.hpp"
#include "draft/audio/sound.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    /**
     * @brief Attaches a Sound to an entity, AudioSystem keeps its position in sync with the
     * entity's TransformComponent.
     */
    struct SoundComponent {
        // Vars
        Sound sound;

        // Constructors
        SoundComponent(){}
        SoundComponent(Sound& sound) : sound(sound) {}

        // Ops
        operator Sound&(){ return sound; }

        DRAFT_REFLECTABLE(SoundComponent, sound)
    };

    /**
     * @brief Attaches a non-owning Music* to an entity (see MusicStorage for ownership).
     */
    struct MusicComponent {
        // Vars
        Resource<Music> track;

        // Constructors
        MusicComponent(){}
        MusicComponent(Resource<Music> track) : track(track) {}

        // Ops
        operator Music*(){ return track.get(); }

        DRAFT_REFLECTABLE(MusicComponent, track)
    };

    /**
     * @brief Marks an entity as an audio listener, its TransformComponent drives the
     * Listener's position/orientation each frame (see AudioSystem).
     */
    struct ListenerComponent {
        // Vars
        Listener listener;
        uint priority = 0; // Higher priority means more precedence. Must be higher than 0 to enable the doppler effect

        // Constructors
        ListenerComponent(){}
        ListenerComponent(Listener& listener) : listener(listener) {}

        // Ops
        operator Listener&(){ return listener; }

        DRAFT_REFLECTABLE(ListenerComponent, listener, priority)
    };
}
