#pragma once

#include "draft/interface/panel.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"

namespace Draft {
    class Rectangle : public Panel {
    private:
        Rect<float> bounds;

    public:
        Rectangle(float x, float y, float w, float h);

        virtual bool handle_event(const Event& event);
        void move(const Vector2f& position);
        void set_color(const Vector4f& color);
    };
};