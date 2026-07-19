#pragma once

#include "draft/ecs/entity.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    /**
     * @brief Finds the best entity at @p worldPoint for click-to-select
     * @return An invalid Entity if nothing at worldPoint matched either pass.
     */
    Entity pick_entity(Scene& scene, const Vector2f& worldPoint);
}
