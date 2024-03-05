#ifndef DRAFT_SYSTEMS_H
#define DRAFT_SYSTEMS_H

#include "SFML/Graphics/RenderWindow.hpp"
#include "draft/phys/world.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"

namespace Draft {
    void renderSystem(entt::registry& registry, sf::RenderWindow* window);
    void physicsSystem(entt::registry& registry, World& world);
}

#endif