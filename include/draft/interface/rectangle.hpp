#pragma once

#include "draft/interface/panel.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    class Rectangle : public Panel {
    public:
        Rectangle(float x, float y, float w, float h);

        void move(const Vector2f& position);
        void set_color(const Vector4f& color);
    };
};