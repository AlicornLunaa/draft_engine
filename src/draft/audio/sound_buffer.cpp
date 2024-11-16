#include "draft/audio/sound_buffer.hpp"
#include "SFML/Audio/SoundBuffer.hpp"
#include <memory>

namespace Draft {
    // Impl
    struct SoundBuffer::Impl {
        sf::SoundBuffer buffer;
    };

    // Private functions
    const void* SoundBuffer::get_buffer_ptr() const {
        return (void*)&ptr->buffer;
    }

    // Constructors
    SoundBuffer::SoundBuffer(const FileHandle& handle) : ptr(std::make_unique<Impl>()) {
        // Load bytes into the buffer
        auto bytes = handle.read_bytes();
        ptr->buffer.loadFromMemory(bytes.data(), bytes.size());
    }

    SoundBuffer::SoundBuffer(const std::vector<std::byte>& rawData){
        ptr->buffer.loadFromMemory(rawData.data(), rawData.size());
    }

    SoundBuffer::SoundBuffer(const SoundBuffer& other) : ptr(std::make_unique<Impl>()) {
        // Copy the other buffer
        ptr->buffer = sf::SoundBuffer(other.ptr->buffer);
    }

    SoundBuffer::~SoundBuffer(){}

    // Operators
    SoundBuffer& SoundBuffer::operator=(const SoundBuffer& other){
        if(this != &other)
            ptr->buffer = other.ptr->buffer;
        
        return *this;
    }

    // Functions
    const int16_t* SoundBuffer::get_samples() const { return ptr->buffer.getSamples(); }
    unsigned int SoundBuffer::get_sample_count() const { return ptr->buffer.getSampleCount(); }
    unsigned int SoundBuffer::get_sample_rate() const { return ptr->buffer.getSampleRate(); }
    unsigned int SoundBuffer::get_channel_count() const { return ptr->buffer.getChannelCount(); }
    Time SoundBuffer::get_duration() const { return Time::microseconds(ptr->buffer.getDuration().asMicroseconds()); }

    void SoundBuffer::reload() {
        auto bytes = handle.read_bytes();
        ptr->buffer.loadFromMemory(bytes.data(), bytes.size());
    }
};