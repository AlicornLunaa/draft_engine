#include "draft/phys/world.hpp"

#include "box2d/b2_world.h"
#include "draft/math/vector2_p.hpp"
#include <memory>

namespace Draft {
    // pImpl
    struct World::Impl {
        // http://louis-langholtz.github.io/Box2D/API/classbox2d_1_1_world.html
        b2World world = b2World({ 0, 0 });
    };

    // Constructors
    World::World(const Vector2f& gravity) : ptr(std::make_unique<Impl>()) {
        ptr->world.SetGravity(vector_to_b2(gravity));
    }

    World::~World(){}

    // Functions

};