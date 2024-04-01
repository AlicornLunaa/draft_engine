#ifndef DRAFT_SYSTEMS_H
#define DRAFT_SYSTEMS_H

#include "draft/phys/world.hpp"
#include "draft/rendering/render_window.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace Draft {
    void renderSystem(entt::registry& registry, RenderWindow& window);
    void physicsSystem(entt::registry& registry, World& world);
}

#endif