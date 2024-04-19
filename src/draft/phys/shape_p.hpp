#pragma once

#include "box2d/b2_polygon_shape.h"
#include "draft/phys/shape.hpp"

namespace Draft {
    b2PolygonShape shape_to_b2(const PolygonShape& shape);
    b2CircleShape shape_to_b2(const CircleShape& shape);
    b2EdgeShape shape_to_b2(const EdgeShape& shape);
};