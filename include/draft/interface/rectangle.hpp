#pragma once

#include "draft/interface/panel.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    class Rectangle : public Panel {
    public:
        Rectangle();

        void move(const Vector2f& position);
        void set_color(const Vector4f& color);
    };
};