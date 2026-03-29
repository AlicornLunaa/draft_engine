#pragma once

#include "draft/math/glm.hpp"
#include "draft/phys/shapes/shape.hpp"
#include <functional>

namespace Draft {
    class Fixture;

    struct RaycastProps {
        Vector2f origin = {0.f, 0.f};
        Vector2f translation = {0.f, 0.f};
        float maxFraction = 1.f;
    };

    struct ShapecastProps {
        Shape& shape;
        Vector2f direction;
        Vector2f originPos;
        Vector2f translationPos;
        float originRot;
        float translationRot;
    };

    struct RaycastResult {
        Vector2f normal = {0.f, 0.f};
        float fraction = 0.f;
    };

    typedef std::function<float(Fixture* fixture, const Vector2f& point, const Vector2f& normal, float fraction)> RaycastCallback;
};