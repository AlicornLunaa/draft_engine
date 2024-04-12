#pragma once

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
        void step(float timeStep, int32_t velocityIterations, int32_t positionIterations);
        
    private:
        // pImpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};