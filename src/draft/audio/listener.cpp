#include "draft/audio/listener.hpp"
#include "SFML/Audio/Listener.hpp"

namespace Draft {
    // Constructors
    Listener::Listener(const Vector3f& position, float volume) : m_position(position), m_volume(volume) {}

    // Functions
    void Listener::set_position(const Vector3f& position){ m_position = position; }
    void Listener::set_direction(const Vector3f& direction){ m_forward = direction; }
    void Listener::set_up(const Vector3f& up){ m_up = up; }
    void Listener::set_volume(float volume){ m_volume = volume; }

    const Vector3f& Listener::get_position(){ return m_position; }
    const Vector3f& Listener::get_direction(){ return m_forward; }
    const Vector3f& Listener::get_up(){ return m_up; }
    float Listener::get_volume(){ return m_volume; }

    void Listener::apply(){
        sf::Listener::setPosition(m_position.x, m_position.y, m_position.z);
        sf::Listener::setDirection(m_forward.x, m_forward.y, m_forward.z);
        sf::Listener::setUpVector(m_up.x, m_up.y, m_up.z);
        sf::Listener::setGlobalVolume(m_volume);
    }
};