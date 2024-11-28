#pragma once

#include "draft/audio/listener.hpp"
#include "draft/audio/music.hpp"
#include "draft/audio/sound.hpp"

namespace Draft {
    struct SoundComponent {
        // Vars
        Sound sound;

        // Constructors
        SoundComponent(Sound& sound) : sound(sound) {};

        // Ops
        operator Sound&(){ return sound; }
    };

    struct MusicComponent {
        // Vars
        Music* music;

        // Constructors
        MusicComponent(Music* music) : music(music) {};

        // Ops
        operator Music*(){ return music; }
    };

    struct ListenerComponent {
        // Vars
        Listener listener;

        // Constructors
        ListenerComponent(){};
        ListenerComponent(Listener& listener) : listener(listener) {};

        // Ops
        operator Listener&(){ return listener; }
    };
};