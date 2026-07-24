#pragma once

#include "draft/asset/resource.hpp"
#include "draft/audio/listener.hpp"
#include "draft/audio/music.hpp"
#include "draft/audio/sound.hpp"
#include "draft/util/reflectable.hpp"
#include "draft/util/serialization/serializer.hpp"

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

        // Serialization (Sound's playback state lives behind get_X/set_X, not plain fields)
        static void serialize(const SoundComponent& component, Binary::ByteArray& out){
            const Sound& sound = component.sound;
            Serializer::serialize(sound.get_buffer(), out);
            Serializer::serialize(sound.get_loop(), out);
            Serializer::serialize(sound.get_playing_offset().as_seconds(), out);
            Serializer::serialize(sound.get_pitch(), out);
            Serializer::serialize(sound.get_volume(), out);
            Serializer::serialize(sound.get_position(), out);
            Serializer::serialize(sound.get_min_distance(), out);
            Serializer::serialize(sound.get_attenuation(), out);
            Serializer::serialize(sound.is_relative(), out);
        }

        static void deserialize_and_advance(SoundComponent& component, Binary::ByteView& span){
            Sound& sound = component.sound;

            Resource<SoundBuffer> buffer;
            Serializer::deserialize_and_advance(buffer, span);
            sound.set_buffer(buffer);

            bool loop;
            Serializer::deserialize_and_advance(loop, span);
            sound.set_loop(loop);

            float offsetSeconds;
            Serializer::deserialize_and_advance(offsetSeconds, span);
            sound.set_playing_offset(Time::seconds(offsetSeconds));

            float pitch;
            Serializer::deserialize_and_advance(pitch, span);
            sound.set_pitch(pitch);

            float volume;
            Serializer::deserialize_and_advance(volume, span);
            sound.set_volume(volume);

            Vector3f position;
            Serializer::deserialize_and_advance(position, span);
            sound.set_position(position);

            float minDistance;
            Serializer::deserialize_and_advance(minDistance, span);
            sound.set_min_distance(minDistance);

            float attenuation;
            Serializer::deserialize_and_advance(attenuation, span);
            sound.set_attenuation(attenuation);

            bool relative;
            Serializer::deserialize_and_advance(relative, span);
            sound.set_relative(relative);
        }

        static void deserialize(SoundComponent& component, Binary::ByteView span){
            deserialize_and_advance(component, span);
        }

        static void serialize(const SoundComponent& component, JSON& json){
            const Sound& sound = component.sound;
            Serializer::serialize(sound.get_buffer(), json["buffer"]);
            json["loop"] = sound.get_loop();
            json["playing_offset"] = sound.get_playing_offset().as_seconds();
            json["pitch"] = sound.get_pitch();
            json["volume"] = sound.get_volume();
            Serializer::serialize(sound.get_position(), json["position"]);
            json["min_distance"] = sound.get_min_distance();
            json["attenuation"] = sound.get_attenuation();
            json["relative"] = sound.is_relative();
        }

        static void deserialize(SoundComponent& component, const JSON& json){
            Sound& sound = component.sound;

            Resource<SoundBuffer> buffer;
            Serializer::deserialize(buffer, json.at("buffer"));
            sound.set_buffer(buffer);

            sound.set_loop(json.at("loop").get<bool>());
            sound.set_playing_offset(Time::seconds(json.at("playing_offset").get<float>()));
            sound.set_pitch(json.at("pitch").get<float>());
            sound.set_volume(json.at("volume").get<float>());

            Vector3f position;
            Serializer::deserialize(position, json.at("position"));
            sound.set_position(position);

            sound.set_min_distance(json.at("min_distance").get<float>());
            sound.set_attenuation(json.at("attenuation").get<float>());
            sound.set_relative(json.at("relative").get<bool>());
        }

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

        // Serialization (Music's playback state lives behind get_X/set_X, not plain fields)
        static void serialize(const MusicComponent& component, Binary::ByteArray& out){
            Serializer::serialize(component.track, out);

            Music* music = component.track.get();
            bool valid = music != nullptr;
            Serializer::serialize(valid, out);
            if(!valid) return;

            Serializer::serialize(music->get_loop(), out);
            Serializer::serialize(music->get_playing_offset().as_seconds(), out);
            Serializer::serialize(music->get_pitch(), out);
            Serializer::serialize(music->get_volume(), out);
            Serializer::serialize(music->get_position(), out);
            Serializer::serialize(music->get_min_distance(), out);
            Serializer::serialize(music->get_attenuation(), out);
            Serializer::serialize(music->is_relative(), out);

            auto [loopStart, loopEnd] = music->get_loop_points();
            Serializer::serialize(loopStart.as_seconds(), out);
            Serializer::serialize(loopEnd.as_seconds(), out);
        }

        static void deserialize_and_advance(MusicComponent& component, Binary::ByteView& span){
            Serializer::deserialize_and_advance(component.track, span);

            bool valid;
            Serializer::deserialize_and_advance(valid, span);
            if(!valid) return;

            bool loop;
            Serializer::deserialize_and_advance(loop, span);
            float offsetSeconds;
            Serializer::deserialize_and_advance(offsetSeconds, span);
            float pitch;
            Serializer::deserialize_and_advance(pitch, span);
            float volume;
            Serializer::deserialize_and_advance(volume, span);
            Vector3f position;
            Serializer::deserialize_and_advance(position, span);
            float minDistance;
            Serializer::deserialize_and_advance(minDistance, span);
            float attenuation;
            Serializer::deserialize_and_advance(attenuation, span);
            bool relative;
            Serializer::deserialize_and_advance(relative, span);
            float loopStartSeconds;
            Serializer::deserialize_and_advance(loopStartSeconds, span);
            float loopEndSeconds;
            Serializer::deserialize_and_advance(loopEndSeconds, span);

            Music* music = component.track.get();
            if(!music) return;

            music->set_loop(loop);
            music->set_playing_offset(Time::seconds(offsetSeconds));
            music->set_pitch(pitch);
            music->set_volume(volume);
            music->set_position(position);
            music->set_min_distance(minDistance);
            music->set_attenuation(attenuation);
            music->set_relative(relative);
            music->set_loop_points(Time::seconds(loopStartSeconds), Time::seconds(loopEndSeconds));
        }

        static void deserialize(MusicComponent& component, Binary::ByteView span){
            deserialize_and_advance(component, span);
        }

        static void serialize(const MusicComponent& component, JSON& json){
            Serializer::serialize(component.track, json["track"]);

            Music* music = component.track.get();
            if(!music) return;

            json["loop"] = music->get_loop();
            json["playing_offset"] = music->get_playing_offset().as_seconds();
            json["pitch"] = music->get_pitch();
            json["volume"] = music->get_volume();
            Serializer::serialize(music->get_position(), json["position"]);
            json["min_distance"] = music->get_min_distance();
            json["attenuation"] = music->get_attenuation();
            json["relative"] = music->is_relative();

            auto [loopStart, loopEnd] = music->get_loop_points();
            json["loop_start"] = loopStart.as_seconds();
            json["loop_end"] = loopEnd.as_seconds();
        }

        static void deserialize(MusicComponent& component, const JSON& json){
            Serializer::deserialize(component.track, json.at("track"));

            Music* music = component.track.get();
            if(!music || !json.contains("loop")) return;

            music->set_loop(json.at("loop").get<bool>());
            music->set_playing_offset(Time::seconds(json.at("playing_offset").get<float>()));
            music->set_pitch(json.at("pitch").get<float>());
            music->set_volume(json.at("volume").get<float>());

            Vector3f position;
            Serializer::deserialize(position, json.at("position"));
            music->set_position(position);

            music->set_min_distance(json.at("min_distance").get<float>());
            music->set_attenuation(json.at("attenuation").get<float>());
            music->set_relative(json.at("relative").get<bool>());
            music->set_loop_points(
                Time::seconds(json.at("loop_start").get<float>()),
                Time::seconds(json.at("loop_end").get<float>())
            );
        }

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
