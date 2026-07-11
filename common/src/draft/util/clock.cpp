#include "draft/util/clock.hpp"

namespace Draft {
    Clock::Clock() : m_start(std::chrono::steady_clock::now()) {}

    Time Clock::get_elapsed_time() const {
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_start);
        return Time::microseconds(elapsed.count());
    }

    Time Clock::restart(){
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - m_start);
        m_start = now;
        return Time::microseconds(elapsed.count());
    }
};
