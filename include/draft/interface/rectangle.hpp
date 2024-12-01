#pragma once

#include "draft/interface/panel.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    class Rectangle : public Panel {
    private:
        // Vars
        Vector4f color;

    public:
        // Constructor
        Rectangle(float x, float y, float w, float h, Panel* parent = nullptr);

        // Functions
        void move(const Vector2f& position);
        void set_color(const Vector4f& color);

        virtual void paint(const Time& deltaTime, SpriteBatch& batch) override;
    };
};