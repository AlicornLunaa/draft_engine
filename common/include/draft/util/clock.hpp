#pragma once

#include "draft/util/time.hpp"
#include <chrono>

namespace Draft {
    /**
     * @brief Measures elapsed wall-clock time from construction (or the last restart()).
     */
    class Clock {
    public:
        /**
         * @brief Starts the clock immediately.
         */
        Clock();

        /**
         * @brief Gets the time elapsed since construction or the last restart(), without
         * resetting it.
         */
        Time get_elapsed_time() const;

        /**
         * @brief Resets the clock to zero.
         * @return The time elapsed since construction or the last restart().
         */
        Time restart();

    private:
        std::chrono::steady_clock::time_point m_start;
    };
};
