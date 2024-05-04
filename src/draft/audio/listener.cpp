#include "draft/audio/listener.hpp"
#include "SFML/Audio/Listener.hpp"

namespace Draft {
    // Constructors
    Listener::Listener(const Vector3f& position, float volume) : position(position), volume(volume) {}

    // Functions
    void Listener::set_position(const Vector3f& position){ this->position = position; }
    void Listener::set_direction(const Vector3f& direction){ this->forward = direction; }
    void Listener::set_up(const Vector3f& up){ this->up = up; }
    void Listener::set_volume(float volume){ this->volume = volume; }

    const Vector3f& Listener::get_position(){ return position; }
    const Vector3f& Listener::get_direction(){ return forward; }
    const Vector3f& Listener::get_up(){ return up; }
    float Listener::get_volume(){ return volume; }

    void Listener::apply(){
        sf::Listener::setPosition(position.x, position.y, position.z);
        sf::Listener::setDirection(forward.x, forward.y, forward.z);
        sf::Listener::setUpVector(up.x, up.y, up.z);
        sf::Listener::setGlobalVolume(volume);
    }
};