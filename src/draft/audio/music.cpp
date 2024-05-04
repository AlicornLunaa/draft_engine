#include "draft/audio/music.hpp"
#include "SFML/Audio/Music.hpp"

#include <memory>

namespace Draft {
    // Impl
    struct Music::Impl {
        sf::Music music;
    };

    // Constructors
    Music::Music() : ptr(std::make_unique<Impl>()) {}
    Music::Music(const std::string& path) : Music() { load(path); }
    Music::~Music(){}

    // Functions
    void Music::play(){ ptr->music.play(); }
    void Music::pause(){ ptr->music.pause(); }
    void Music::stop(){ ptr->music.stop(); }
    void Music::load(const std::string& path){ ptr->music.openFromFile(path); }

    void Music::set_loop_points(Time start, Time end){ ptr->music.setLoopPoints({ sf::microseconds(start.as_microseconds()), sf::microseconds((end - start).as_microseconds())}); }
    void Music::set_playing_offset(Time timeOffset){ ptr->music.setPlayingOffset(sf::microseconds(timeOffset.as_microseconds())); }
    void Music::set_pitch(float pitch){ ptr->music.setPitch(pitch); }
    void Music::set_volume(float volume){ ptr->music.setVolume(volume); }
    void Music::set_min_distance(float distance){ ptr->music.setMinDistance(distance); }
    void Music::set_attenuation(float attenuation){ ptr->music.setAttenuation(attenuation); }
    void Music::set_loop(bool loop){ ptr->music.setLoop(loop); }
    void Music::set_relative(bool relative){ ptr->music.setRelativeToListener(relative); }
    void Music::set_position(const Vector3f& position){ ptr->music.setPosition(position.x, position.y, position.z); }

    unsigned int Music::get_sample_rate() const { return ptr->music.getSampleRate(); }
    unsigned int Music::get_channel_count() const { return ptr->music.getChannelCount(); }
    Music::TimeSpan Music::get_loop_points() const { auto span = ptr->music.getLoopPoints(); return {Time::microseconds(span.offset.asMicroseconds()), Time::microseconds((span.offset + span.length).asMicroseconds())}; }
    Time Music::get_playing_offset() const { return Time::microseconds(ptr->music.getPlayingOffset().asMicroseconds()); }
    Time Music::get_duration() const { return Time::microseconds(ptr->music.getDuration().asMicroseconds()); };
    float Music::get_pitch() const { return ptr->music.getPitch(); };
    float Music::get_volume() const { return ptr->music.getVolume(); }
    float Music::get_min_distance() const { return ptr->music.getMinDistance(); }
    float Music::get_attenuation() const { return ptr->music.getAttenuation(); }
    bool Music::get_loop() const { return ptr->music.getLoop(); }
    bool Music::is_relative() const { return ptr->music.isRelativeToListener(); }
    Vector3f Music::get_position() const { auto p = ptr->music.getPosition(); return {p.x, p.y, p.z}; }
};