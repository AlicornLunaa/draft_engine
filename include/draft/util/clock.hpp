#pragma once

#include <memory>

#include "draft/util/time.hpp"

namespace Draft {
    class Clock {
    public:
        // Constructors
        Clock();
        ~Clock();

        // Functions
        Time get_elapsed_time() const;
        Time restart();

    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};