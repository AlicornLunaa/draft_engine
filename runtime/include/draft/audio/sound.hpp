#pragma once

#include "draft/asset/resource.hpp"
#include "draft/audio/sound_buffer.hpp"
#include "draft/math/glm.hpp"
#include "draft/util/reflectable.hpp"
#include "draft/util/time.hpp"
#include <memory>

namespace Draft {
    /**
     * @brief A single playable instance of a SoundBuffer, positioned, pitched, and looped
     * independently of any other Sound sharing the same buffer.
     */
    class Sound {
    public:
        // Constructors
        Sound();
        Sound(Resource<SoundBuffer> buffer);
        Sound(const Sound& other);
        ~Sound();

        // Operators
        Sound& operator=(const Sound& other);

        // Functions
        void play();
        void pause();
        void stop();
        void reset_buffer();

        void set_buffer(Resource<SoundBuffer> buffer);
        void set_loop(bool loop);
        void set_playing_offset(Time timeOffset);
        void set_pitch(float pitch);
        void set_volume(float volume);
        void set_position(const Vector3f& position);
        void set_min_distance(float distance);
        void set_attenuation(float attenuation);
        void set_relative(bool relative);

        Resource<SoundBuffer> get_buffer() const;
        bool get_loop() const;
        Time get_playing_offset() const;
        float get_pitch() const;
        float get_volume() const;
        Vector3f get_position() const;
        float get_min_distance() const;
        float get_attenuation() const;
        bool is_relative() const;
        inline bool is_playing() const { return m_isPlaying; }
        inline bool is_paused() const { return m_isPaused; }

        DRAFT_REFLECTABLE(Sound, buffer)

    private:
        struct Impl;
        std::unique_ptr<Impl> ptr;

        Resource<SoundBuffer> buffer;
        bool m_isPlaying = false;
        bool m_isPaused = false;
    };
}
