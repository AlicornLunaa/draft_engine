#pragma once

#include "draft/rendering/camera.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    /**
     * @brief Marks an entity as a candidate for a Scene's active camera.
     */
    struct CameraComponent {
        bool  active = true;
        int  priority = 0;
        Camera camera;

        DRAFT_REFLECTABLE(CameraComponent, active, priority, camera)
    };
}
