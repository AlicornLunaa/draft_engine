#include "draft/audio/sound.hpp"
#include "SFML/Audio/Sound.hpp"
#include "SFML/System/Time.hpp"
#include <memory>

namespace Draft {
    // Impl
    struct Sound::Impl {
        sf::Sound sound;
    };

    // Constructors
    Sound::Sound() : ptr(std::make_unique<Impl>()), m_bufferPtr() {}

    Sound::Sound(const SoundBuffer& buffer) : Sound() { set_buffer(buffer); }

    Sound::Sound(const Sound& other) : Sound() { ptr->sound = sf::Sound(other.ptr->sound); }

    Sound::~Sound(){}

    // Operators
    Sound& Sound::operator=(const Sound& other){
        if(this != &other)
            ptr->sound = other.ptr->sound;

        return *this;
    }

    // Functions
    void Sound::play(){ ptr->sound.play(); }
    void Sound::pause(){ ptr->sound.pause(); }
    void Sound::stop(){ ptr->sound.stop(); }
    void Sound::reset_buffer(){ ptr->sound.resetBuffer(); }

    void Sound::set_buffer(const SoundBuffer& buffer){
        m_bufferPtr = &buffer;
        ptr->sound.setBuffer(*((const sf::SoundBuffer*)buffer.get_buffer_ptr()));
    }

    void Sound::set_loop(bool loop){ ptr->sound.setLoop(loop); }
    void Sound::set_playing_offset(Time timeOffset){ ptr->sound.setPlayingOffset(sf::microseconds(timeOffset.as_microseconds())); }
    void Sound::set_pitch(float pitch){ ptr->sound.setPitch(pitch); }
    void Sound::set_volume(float volume){ ptr->sound.setVolume(volume); }
    void Sound::set_position(const Vector3f& position){ ptr->sound.setPosition({ position.x, position.y, position.z }); }
    void Sound::set_min_distance(float distance){ ptr->sound.setMinDistance(distance); }
    void Sound::set_attenuation(float attenuation){ ptr->sound.setAttenuation(attenuation); }
    void Sound::set_relative(bool relative){ ptr->sound.setRelativeToListener(relative); }

    const SoundBuffer* Sound::get_buffer() const { return m_bufferPtr; }
    bool Sound::get_loop() const { return ptr->sound.getLoop(); }
    Time Sound::get_playing_offset() const { return Time::microseconds(ptr->sound.getPlayingOffset().asMicroseconds()); }
    float Sound::get_pitch() const { return ptr->sound.getPitch(); }
    float Sound::get_volume() const { return ptr->sound.getVolume(); }
    Vector3f Sound::get_position() const { auto p = ptr->sound.getPosition(); return { p.x, p.y, p.z }; }
    float Sound::get_min_distance() const { return ptr->sound.getMinDistance(); }
    float Sound::get_attenuation() const { return ptr->sound.getAttenuation(); }
    bool Sound::is_relative() const { return ptr->sound.isRelativeToListener(); }
};