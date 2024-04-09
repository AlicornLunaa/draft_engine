#include "draft/util/clock.hpp"
#include "SFML/System/Clock.hpp"
#include <memory>

namespace Draft {
    // Implementation
    struct Clock::Impl {
        sf::Clock clock;
    };

    // Constructors
    Clock::Clock() : ptr(std::make_unique<Impl>()) {}
    Clock::~Clock(){}

    // Functions
    Time Clock::get_elapsed_time() const { return Time::microseconds(ptr->clock.getElapsedTime().asMicroseconds()); }
    Time Clock::restart(){ return Time::microseconds(ptr->clock.restart().asMicroseconds()); }
};