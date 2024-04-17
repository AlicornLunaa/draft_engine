#pragma once

#include "box2d/b2_world_callbacks.h"
#include "draft/math/vector2.hpp"
#include <memory>

namespace Draft {
    class World {
    public:
        // Constructors
        World(const Vector2f& gravity);
        World(const World& other) = delete;
        ~World();

        // Functions
        void set_destruction_listener(void* listener) noexcept;
        void step(float timeStep, int32_t velocityIterations, int32_t positionIterations);
        
    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};