#ifndef DRAFT_SYSTEMS_H
#define DRAFT_SYSTEMS_H

#include "draft/core/application.hpp"
#include "draft/core/registry.hpp"
#include "draft/phys/world.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/rendering/sprite_batch.hpp"

namespace Draft {
    void render_system(Registry& registry, SpriteBatch& batch, RenderWindow& window, const Camera* camera = nullptr);
    void physics_system(Registry& registry, const Application* app, World& world);
}

#endif