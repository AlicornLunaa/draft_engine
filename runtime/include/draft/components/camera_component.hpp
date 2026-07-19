#pragma once

#include "draft/rendering/camera.hpp"
#include "draft/util/reflectable.hpp"

#include <memory>

namespace Draft {
    /**
     * @brief Marks an entity as a candidate for a Scene's active camera.
     */
    struct CameraComponent {
        DRAFT_REFLECTED(bool, active) = true;
        DRAFT_REFLECTED(int, priority) = 0;
        std::unique_ptr<Camera> camera;

        DRAFT_REFLECTABLE(CameraComponent, active, priority)
    };
}
