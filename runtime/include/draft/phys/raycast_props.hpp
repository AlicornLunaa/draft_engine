#pragma once

#include "draft/math/glm.hpp"
#include "draft/phys/shapes/shape.hpp"
#include "draft/util/reflectable.hpp"

#include <functional>

namespace Draft {
    class Fixture;

    /**
     * @brief Query parameters for a straight-line raycast from `origin`, along `translation`,
     * stopping past `maxFraction` of that translation.
     */
    struct RaycastProps {
        DRAFT_REFLECTED(Vector2f, origin) = {0.f, 0.f};
        DRAFT_REFLECTED(Vector2f, translation) = {0.f, 0.f};
        DRAFT_REFLECTED(float, maxFraction) = 1.f;

        DRAFT_REFLECTABLE(RaycastProps, origin, translation, maxFraction)
    };

    /**
     * @brief Query parameters for a shape-swept cast from `originPos`/`originRot`, along
     * `direction`, to `translationPos`/`translationRot`.
     */
    struct ShapecastProps {
        Shape& shape;
        Vector2f direction;
        Vector2f originPos;
        Vector2f translationPos;
        float originRot;
        float translationRot;
    };

    /**
     * @brief Result of a raycast hit containing surface normal and the fraction of `RaycastProps::translation`
     * travelled before impact.
     */
    struct RaycastResult {
        DRAFT_REFLECTED(Vector2f, normal) = {0.f, 0.f};
        DRAFT_REFLECTED(float, fraction) = 0.f;

        DRAFT_REFLECTABLE(RaycastResult, normal, fraction)
    };

    using RaycastCallback = std::function<float(Fixture* fixture, const Vector2f& point, const Vector2f& normal, float fraction)>;
}
