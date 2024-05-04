#pragma once

#include "draft/util/file_handle.hpp"
#include "draft/util/time.hpp"
#include <cstdint>
#include <memory>

namespace Draft {
    class Sound;

    class SoundBuffer {
    private:
        // Variables
        FileHandle handle;

        // Private functions
        const void* get_buffer_ptr() const;

    public:
        // Constructors
        SoundBuffer(const FileHandle& handle);
        SoundBuffer(const SoundBuffer& other);
        ~SoundBuffer();

        // Friends
        friend class Sound;

        // Operators
        SoundBuffer& operator=(const SoundBuffer& other);

        // Functions
        const int16_t* get_samples() const;
        unsigned int get_sample_count() const;
        unsigned int get_sample_rate() const;
        unsigned int get_channel_count() const;
        Time get_duration() const;
        void reload();

    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};