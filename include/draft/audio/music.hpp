#pragma once

#include <memory>
#include <string>
#include <utility>

#include "draft/math/glm.hpp"
#include "draft/util/time.hpp"

namespace Draft {
    class Music {
    private:
        // Types
        typedef std::pair<Time, Time> TimeSpan;

        // Variables
        std::string path;

    public:
        // Constructors
        Music();
        Music(const std::string& path);
        Music(const Music& other) = delete;
        ~Music();

        // Operators
        Music& operator=(const Music& other) = delete;

        // Functions
        void play();
        void pause();
        void stop();
        void load(const std::string& path);

        void set_loop_points(Time start, Time end);
        void set_playing_offset(Time timeOffset);
        void set_pitch(float pitch);
        void set_volume(float volume);
        void set_min_distance(float distance);
        void set_attenuation(float attenuation);
        void set_loop(bool loop);
        void set_relative(bool relative);
        void set_position(const Vector3f& position);

        unsigned int get_sample_rate() const;
        unsigned int get_channel_count() const;
        TimeSpan get_loop_points() const;
        Time get_playing_offset() const;
        Time get_duration() const;
        float get_pitch() const;
        float get_volume() const;
        float get_min_distance() const;
        float get_attenuation() const;
        bool get_loop() const;
        bool is_relative() const;
        Vector3f get_position() const;

    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};