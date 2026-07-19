#pragma once

#include "draft/ecs/system.hpp"
#include "draft/math/glm.hpp"
#include "draft/util/reflectable.hpp"

namespace Draft {
    class EditorApplication;

    /**
     * @brief Orthographic pan/zoom navigation camera for the editor viewport.
     */
    class FreecamControllerSystem : public AbstractSystem {
    public:
        explicit FreecamControllerSystem(EditorApplication& app);

        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(FreecamControllerSystem)

    private:
        EditorApplication& m_app;

        Vector2f m_position{0.f, 0.f};
        float m_zoom = 1.f;
        double m_lastScrollY = 0.0;
    };
}
