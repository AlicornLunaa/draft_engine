#pragma once

#include "draft/util/files/file_handle.hpp"
#include "draft/util/time.hpp"
#include <cstdint>
#include <memory>
#include <vector>

namespace Draft {
    class Sound;

    /**
     * @brief Decoded PCM audio data, loaded once from a file or raw bytes and shared by any
     * number of Sound instances
     */
    class SoundBuffer {
    private:
        // Private functions
        const void* get_buffer_ptr() const;

    public:
        // Constructors
        SoundBuffer(const FileHandle& handle);
        SoundBuffer(const std::vector<std::byte>& rawData);
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

    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
}
