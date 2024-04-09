#ifndef DRAFT_SYSTEMS_H
#define DRAFT_SYSTEMS_H

#include "draft/core/registry.hpp"
#include "draft/rendering/render_window.hpp"

namespace Draft {
    void render_system(Registry& registry, RenderWindow& window);
    // void physicsSystem(entt::registry& registry, World& world);
}

#endif