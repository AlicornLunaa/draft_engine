#pragma once

#include "draft/interface/panel.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/camera.hpp"

namespace Draft {
    class Rectangle : public Panel {
    private:
        const OrthographicCamera& camera;
        Rect<float> bounds;

    public:
        Rectangle(const OrthographicCamera& camera);

        virtual bool handle_event(const Event& event);
        void move(const Vector2f& position);
        void set_color(const Vector4f& color);
    };
};