#include "draft/interface/rectangle.hpp"
#include "draft/math/glm.hpp"

namespace Draft {
    // Constructor
    Rectangle::Rectangle(float x, float y, float w, float h, Panel* parent) : Panel(parent) {
        color = Vector4f(0.2, 0.2, 0.2, 1.0);
        bounds.x = x;
        bounds.y = y;
        bounds.width = w;
        bounds.height = h;
    }

    // Functions
    void Rectangle::move(const Vector2f& position){
        bounds.x += position.x;
        bounds.y += position.y;
    }

    void Rectangle::set_color(const Vector4f& color){
        this->color = color;
    }

    void Rectangle::paint(const Time& deltaTime, SpriteBatch& batch){
        batch.draw({
            nullptr,
            {},
            {bounds.x, bounds.y},
            0.f,
            {bounds.width, bounds.height},
            {bounds.width * 0.5f, bounds.height * 0.5f},
            0.f,
            color,
            false
        });
    }
};