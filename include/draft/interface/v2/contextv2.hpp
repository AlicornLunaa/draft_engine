#pragma once

#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"

struct Context {
    Draft::FloatRect parentBounds; // Area which children can render to
    const Draft::Vector2u windowSize;
};